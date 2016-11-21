#include <stdlib.h>

#include "RankingCache.h"

RankingCache::RankingCache() {
}

RankingCache::~RankingCache() {
}

void RankingCache::rank(const skullcpp::Service& service, const std::string& question,
        const RankingRecords& records) const {

}

void RankingCache::rankResult(const std::string& question, RankingRecords& records) const {
    const auto iter = this->cache.find(question);
    if (iter == this->cache.end()) return;

    time_t now = time(NULL);
    for (const auto& record : iter->second) {
        if (now >= record.expiredTime_) {
            continue;
        }

        updateRankResult(record.ip_, record.score_, records);
    }
}

bool RankingCache::updateRankResult(const std::string& ip, double score,
        RankingRecords& records) const {
    for (auto& record : records) {
        if (ip != record.ip_) continue;

        record.score_ = score;
        return true;
    }

    return false;
}
