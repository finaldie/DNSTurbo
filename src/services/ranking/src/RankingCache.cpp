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
void _updateHttpInfo(skullcpp::Service& service, const std::string question,
                  const std::string ip, int status, int httpCode, int latency) {
    auto* cache = (RankingCache*)service.get();
    bool ret = cache->updateRankResult(question, ip, status, httpCode, latency);

    SKULLCPP_LOG_DEBUG("Update record: " << ret
                      << " ,question: " << question
                      << " ,ip: "       << ip
                      << " ,latency: "  << latency);
}

static
void _httpResponseCb(const skullcpp::Service& service,
                     skullcpp::EPClientNPRet& ret,
                     const std::string& question,
                     const std::shared_ptr<HttpResponse>& httpResponse) {
    int status   = ret.status();
    int httpCode = httpResponse->statusCode();
    int latency  = ret.latency();

    SKULLCPP_LOG_DEBUG("HttpResponseCb: question: " << question
                      << " ,ip: " << ret.ip()
                      << " ,status: " << status
                      << " ,httpCode: " << httpCode
                      << " ,latency: " << latency);

    service.createJob(0, 1,
        skull_BindSvcJobNPW(_updateHttpInfo, question, ret.ip(), status, httpCode, latency), NULL);
}

RankingCache::RankingCache() {
}

RankingCache::~RankingCache() {
}

void RankingCache::addIntoCache(const std::string& question,
        const RankingRecords& records) {
    if (question.empty()) return;
    if (records.empty()) return;

    auto iter = this->cache.find(question);
    if (iter == this->cache.end()) {
        this->cache.insert(std::make_pair(question, records));
    } else {
        updateCacheRecords(records, iter->second);
    }
}

void RankingCache::rankResult(const std::string& question, RankingRecords& records) const {
    const auto iter = this->cache.find(question);
    if (iter == this->cache.end()) return;
    const auto& config = skullcpp::Config::instance();

    // Filter out the expired records, and fill the score into output records
    time_t now = time(NULL);
    for (const auto& record : iter->second) {
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

const std::string RankingCache::dumpCache() const {
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

bool RankingCache::updateRankResult(const std::string& question,
                                    const std::string& ip,
                                    int status,
                                    int httpCode,
                                    int latency) {
    auto iter = this->cache.find(question);
    if (iter == this->cache.end()) return false;

    for (auto& item : iter->second) {
        if (item.ip_ != ip) {
            continue;
        }

        HttpInfo httpInfo;
        httpInfo.status_   = status;
        httpInfo.httpCode_ = httpCode;
        httpInfo.latency_  = latency;

        if (item.httpInfo_.size() == MAX_HTTP_INFO) {
            item.httpInfo_.erase(item.httpInfo_.begin());
        }
        item.httpInfo_.push_back(httpInfo);
        item.httpInfo_.shrink_to_fit();
        return true;
    }

    return false;
}

int RankingCache::updateCacheRecords(const RankingRecords& latest, RankingRecords& curr) {
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

    time_t now = time(NULL);
    auto iter = this->cache.begin();

    for (; iter != this->cache.end(); iter++) {
        auto riter = iter->second.begin();

        for (; riter != iter->second.end(); ) {
            if (now >= riter->expiredTime_ + delayed) {
                iter->second.erase(riter);
                totalCleaned++;
            } else {
                ++riter;
            }
        }
    }

    return totalCleaned;
}

void RankingCache::doSpeedTest(const skullcpp::Service& service) const {
    // Send http request and collect the status and latency information
    //  Then do the scoring task based on these information

    SKULLCPP_LOG_DEBUG("Speed Test Start...");
    for (const auto& item : this->cache) {
        const auto& question = item.first;
        const auto& records  = item.second;

        for (const auto& record : records) {
            HttpRequest httpReq;
            httpReq.setMethod("GET");
            httpReq.setURI("/");
            bool ret = httpReq.validate();

            SKULLCPP_LOG_DEBUG("doSpeedTest: http request validation: " << ret
                              << " question: " << question
                              << " qtype: "       << record.qtype_
                              << " ip: "          << record.ip_
                              << " expiredtime: " << record.expiredTime_);

            EPHandler handler;
            handler.send(service, record.ip_, 80, 2000, httpReq.getHttpContent(),
                         question, _httpResponseCb);
        }
    }
}

