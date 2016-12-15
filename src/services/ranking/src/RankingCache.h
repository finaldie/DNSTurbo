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
        int status_;
        int httpCode_;
        int latency_;
    } HttpInfo;

    typedef struct RankingRecord {
        std::string ip_;
        QTYPE       qtype_;
        time_t      expiredTime_;

        std::vector<HttpInfo> httpInfo_;
    } RankingRecord;

    typedef std::vector<RankingRecord> RankingRecords;
    typedef std::map<std::string, RankingRecords> RankingMap;

private:
    RankingMap cache;

public:
    RankingCache();
    ~RankingCache();

public:
    void addIntoCache(const std::string& question, const RankingRecords&);

    void rankResult(const std::string& question, RankingRecords&) const;

    int  cleanup();
    int  cleanup(int deplayed);

    void doSpeedTest(const skullcpp::Service& service) const;

    bool updateRankResult(const std::string& question, const std::string& ip,
                          int status, int httpCode, int latency);

    const std::string dump() const;
    const std::string status() const;

private:
    bool updateRankResult(const RankingRecord&, RankingRecords&) const;
    int  updateCacheRecords(const RankingRecords& latest, RankingRecords& curr);
};

#endif

