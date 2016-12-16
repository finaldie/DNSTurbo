#ifndef RANKING_CACHE_H
#define RANKING_CACHE_H

#include <time.h>
#include <string>
#include <map>
#include <vector>

#include <skullcpp/api.h>

class RankingCache : public skullcpp::ServiceData {
public:
    typedef enum QTYPE {
        A    = 1,
        AAAA = 2
    } QTYPE;

    typedef struct HttpInfo {
        HttpInfo() : status_(0), httpCode_(0), latency_(0) {}

        int status_;
        int httpCode_;
        int latency_;
    } HttpInfo;

    typedef struct RankingRecord {
        RankingRecord() : qtype_(A), expiredTime_(0) {}

        std::string ip_;
        QTYPE       qtype_;
        time_t      expiredTime_;

        std::vector<HttpInfo> httpInfo_;
    } RankingRecord;

    typedef struct LatencyRecord {
        LatencyRecord() : qtype_(A) {}

        std::string   question_;
        std::string   ip_;
        QTYPE         qtype_;
        HttpInfo      httpInfo_;
    } LatencyRecord;

    typedef std::vector<RankingRecord> RankingRecords;
    typedef std::map<std::string, RankingRecords> RankingMap;
    typedef std::vector<LatencyRecord> LatencyRecords;

    typedef struct LatencyCache {
        LatencyCache() {}
        void reset() {
            this->qip2idx_.clear();
            this->queue_.clear();
        }

        std::map<std::string, size_t> qip2idx_; // (question, ip) to Index
        LatencyRecords queue_;
    } LatencyCache;

private:
    RankingMap cache;

    // Every certain rounds, the speedTestRecords will be dump into Ranking
    //  cache
    LatencyCache latencyCache;

public:
    RankingCache();
    ~RankingCache();

public:
    void addIntoCache(const skullcpp::Service& service,
                      const std::string& question, const RankingRecords&);

    void rankResult(const std::string& question, RankingRecords&) const;


public:
    int  cleanup();
    int  cleanup(int deplayed);

public:
    void doSpeedTest(const skullcpp::Service& service, size_t start, size_t end) const;
    bool updateLatencyResult(const std::string& question, const std::string& ip,
                               int status, int httpCode, int latency);
    void shipLatencyResults();
    void rebuildLatencyCache();
    size_t latencyCacheSize() const;

public:
    const std::string dump() const;
    const std::string status() const;

private:
    bool updateRankResult(const RankingRecord&, RankingRecords&) const;
    int  updateCacheRecords(const std::string& question,
                            const RankingRecords& latest, RankingRecords& curr);

    void doSpeedTest(const skullcpp::Service& service,
                     const std::string& question,
                     const std::string& ip,
                     QTYPE qtype) const;

    void appendLatencyCache(const std::string& question,
                            const RankingRecord& record);
};

#endif

