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

    typedef struct RankingRecord {
        std::string ip_;
        QTYPE       qtype_;
        time_t      expiredTime_;
        double      score_;
    } RankingRecord;

    typedef std::vector<RankingRecord> RankingRecords;
    typedef std::map<std::string, RankingRecords> RankingMap;

private:
    RankingMap cache;

public:
    RankingCache();
    ~RankingCache();

public:
    void rank(const skullcpp::Service& service, const std::string& question,
            const RankingRecords&) const;

    void rankResult(const std::string& question, RankingRecords&) const;

private:
    bool updateRankResult(const std::string& ip, double score, RankingRecords&) const;
};

#endif

