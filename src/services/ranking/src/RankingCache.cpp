#include <stdlib.h>
#include <memory>
#include <sstream>

#include "config.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "EPHandler.h"
#include "RankingCache.h"

#define MAX_HTTP_INFO 20

static
void _httpResponseCb(const skullcpp::Service& service,
                     skullcpp::EPClientNPRet& ret,
                     const std::string& question,
                     const std::shared_ptr<HttpResponse>& httpResponse) {
    int status   = ret.status();
    int httpCode = httpResponse->statusCode();
    int latency  = ret.latency();
    const std::string& ip = ret.ip();

    SKULLCPP_LOG_DEBUG("HttpResponseCb: question: " << question
                      << " ,ip: "       << ip
                      << " ,status: "   << status
                      << " ,httpCode: " << httpCode
                      << " ,latency: "  << latency);

    service.createJob(0, 1, [=] (skullcpp::Service& service) {
        auto* cache = (RankingCache*)service.get();
        bool ret = cache->updateLatencyResult(question, ip, status, httpCode, latency);

        SKULLCPP_LOG_DEBUG("Update record: " << ret
                           << " ,question: " << question
                           << " ,ip: "       << ip
                           << " ,latency: "  << latency);
    }, [=] (const skullcpp::Service& service) {
        SKULLCPP_LOG_ERROR("_HttpResponseCb", "Update record failed, "
                           << " ,question: " << question
                           << " ,ip: "       << ip
                           << " ,latency: "  << latency,
                           "Reduce system load or increase service queue size");
    });
}

RankingCache::RankingCache() {
}

RankingCache::~RankingCache() {
}

void RankingCache::addIntoCache(const skullcpp::Service& service,
                                const std::string& question,
                                const RankingRecords& records) {
    if (question.empty()) return;
    if (records.empty()) return;

    auto iter = this->cache.find(question);
    if (iter == this->cache.end()) {
        // 1. Insert records into ranking cache
        this->cache.insert(std::make_pair(question, records));

        // 2. Append into speed test array
        for (const auto& record : records) {
            appendLatencyCache(question, record);
        }
    } else {
        updateCacheRecords(question, records, iter->second);
    }
}

void RankingCache::rankResult(const std::string& question, RankingRecords& records) const {
    if (records.empty()) return;

    const auto iter = this->cache.find(question);
    if (iter == this->cache.end()) return;
    const auto& config = skullcpp::Config::instance();

    // Filter out the expired records, and fill the score into output records
    time_t now = time(NULL);
    for (const auto& record : iter->second) {
        if (record.qtype_ != records[0].qtype_) {
            continue;
        }

        if (now - config.cleanup_interval() >= record.expiredTime_) {
            continue;
        }

        updateRankResult(record, records);
    }
}

bool RankingCache::updateRankResult(const RankingRecord& curr,
        RankingRecords& records) const {
    for (auto& record : records) {
        if (curr.ip_ != record.ip_) continue;

        record.httpInfo_ = curr.httpInfo_;
        return true;
    }

    return false;
}

const std::string RankingCache::dump() const {
    std::ostringstream oss;

    for (const auto& qitem : this->cache) {
        oss << "question: " << qitem.first << " ,";

        for (const auto& record : qitem.second) {
            oss << "ip: " << record.ip_ << " ,"
                << "expiredTime: " << record.expiredTime_ << " ";

            oss << "httpInfo: ";
            for (const auto& item : record.httpInfo_) {
                oss << "{" << item.status_ << ", "
                    << item.httpCode_ << ", "
                    << item.latency_ << "}";
            }
        }
    }

    return oss.str();
}

const std::string RankingCache::status() const {
    std::ostringstream oss;
    oss << "Total questions: " << this->cache.size();

    size_t totalRecords = 0;
    for (const auto& qitem : this->cache) {
        totalRecords += qitem.second.size();
    }

    oss << " ,total records: " << totalRecords;
    return oss.str();
}

bool RankingCache::updateLatencyResult(const std::string& question,
                                    const std::string& ip,
                                    int status,
                                    int httpCode,
                                    int latency) {
    // 1. Get Index
    std::string key = question + "__" + ip;
    auto iter = this->latencyCache.qip2idx_.find(key);
    if (iter == this->latencyCache.qip2idx_.end()) return false;

    // 2. Verify Index
    size_t idx = iter->second;
    size_t qsz = this->latencyCache.queue_.size();
    if (idx >= qsz) return false;

    // 3. Update Result
    auto& item = this->latencyCache.queue_[idx];
    item.httpInfo_.status_   = status;
    item.httpInfo_.httpCode_ = httpCode;
    item.httpInfo_.latency_  = latency;
    return true;
}

int RankingCache::updateCacheRecords(const std::string& question,
    const RankingRecords& latest, RankingRecords& curr) {
    int totalUpdates = 0;

    for (const auto& record : latest) {
        bool found = false;
        for (auto& currRecord : curr) {
            if (currRecord.ip_ != record.ip_) {
                continue;
            }

            if (currRecord.expiredTime_ != record.expiredTime_) {
                currRecord.expiredTime_ = record.expiredTime_;
                totalUpdates++;
            }

            found = true;
            break;
        }

        if (!found) {
            curr.push_back(record);
            appendLatencyCache(question, record);

            totalUpdates++;
        }
    }

    return totalUpdates;
}

int RankingCache::cleanup() {
    return cleanup(0);
}

int RankingCache::cleanup(int delayed) {
    int totalCleaned = 0;
    int total = 0;

    time_t now = time(NULL);
    auto iter = this->cache.begin();

    for (; iter != this->cache.end(); ) {
        auto riter = iter->second.begin();
        total += iter->second.size();

        // Clean the expired ranking records
        for (; riter != iter->second.end(); ) {
            if (now >= riter->expiredTime_ + delayed) {
                riter = iter->second.erase(riter);
                totalCleaned++;
            } else {
                ++riter;
            }
        }

        // Clean the key if the rankingRecord vector is empty
        if (iter->second.empty()) {
            iter = this->cache.erase(iter);
        } else {
            ++iter;
        }
    }

    SKULLCPP_LOG_INFO("RankingCache", "Clean up " << totalCleaned << " records"
                      << ", Total: " << total);
    return totalCleaned;
}

void RankingCache::doSpeedTest(const skullcpp::Service& service, size_t start,
                               size_t end) const {
    // Send http request and collect the status and latency information
    //  Then do the scoring task based on these information
    SKULLCPP_LOG_DEBUG("Speed Test Start...");

    size_t sz = this->latencyCache.queue_.size();
    if (end < start || end >= sz) {
        return;
    }

    for (size_t i = start; i <= end; i++) {
        const auto& item = this->latencyCache.queue_[i];
        doSpeedTest(service, item.question_, item.ip_, item.qtype_);
    }
}

void RankingCache::doSpeedTest(const skullcpp::Service& service,
                               const std::string& question,
                               const std::string& ip,
                               QTYPE qtype) const {
    HttpRequest httpReq;
    httpReq.setMethod("GET");
    httpReq.setURI("/");
    bool ret = httpReq.validate();

    SKULLCPP_LOG_DEBUG("doSpeedTest: http request validation: " << ret
                      << " question: "    << question
                      << " ip: "          << ip
                      << " qtype: "       << qtype);

    EPHandler handler;
    handler.send(service, ip, 80, 2000, httpReq.getHttpContent(),
                 question, _httpResponseCb);
}

void RankingCache::appendLatencyCache(const std::string& question,
                                      const RankingRecord& record) {
    LatencyRecord sRecord;
    sRecord.question_ = question;
    sRecord.ip_       = record.ip_;
    sRecord.qtype_    = record.qtype_;

    // Push to queue
    size_t sz = this->latencyCache.queue_.size();
    this->latencyCache.queue_.push_back(sRecord);

    // Create mapping
    std::string key = question + "__" + sRecord.ip_;
    this->latencyCache.qip2idx_.insert(std::make_pair(key, sz));
}

void RankingCache::shipLatencyResults() {
    for (const auto& item : this->latencyCache.queue_) {
        const std::string& question = item.question_;
        const std::string& ip       = item.ip_;
        QTYPE qtype                 = item.qtype_;
        const HttpInfo& httpInfo    = item.httpInfo_;

        auto iter = this->cache.find(question);
        if (iter == this->cache.end()) continue;

        for (auto& record : iter->second) {
            if (record.ip_ != ip || record.qtype_ != qtype) {
                continue;
            }

            if (record.httpInfo_.size() == MAX_HTTP_INFO) {
                record.httpInfo_.erase(record.httpInfo_.begin());
            }

            record.httpInfo_.push_back(httpInfo);
            record.httpInfo_.shrink_to_fit();
        }
    }
}

void RankingCache::rebuildLatencyCache() {
    this->latencyCache.reset();

    auto iter = this->cache.cbegin();
    for (; iter != this->cache.end(); iter++) {
        const std::string& question = iter->first;
        const auto& records = iter->second;

        for (const auto& record : records) {
            appendLatencyCache(question, record);
        }
    }
}

size_t RankingCache::latencyCacheSize() const {
    return this->latencyCache.queue_.size();
}

