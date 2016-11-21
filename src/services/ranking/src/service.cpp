#include <stdlib.h>

#include <iostream>
#include <string>
#include <google/protobuf/message.h>

#include <skullcpp/api.h>
#include "skull_protos.h"
#include "config.h"

#include "RankingCache.h"

using namespace skull::service::ranking;

// ====================== Service Init/Release =================================
/**
 * Service Initialization. It will be called before module initialization.
 */
static
void skull_service_init(skullcpp::Service& service, const skull_config_t* config)
{
    SKULLCPP_LOG_INFO("svc.ranking", "Skull service initializing");

    // Load skull_config to skullcpp::Config
    skullcpp::Config::instance().load(config);

    // Set Ranking Cache
    service.set(new RankingCache());
}

/**
 * Service Release
 */
static
void skull_service_release(skullcpp::Service& service)
{
    SKULLCPP_LOG_INFO("svc.ranking", "Skull service releasd");
}

/**************************** Service APIs Calls *******************************
 *
 * Service API implementation. For the api which has read access, you can call
 *  `service.get()` to fetch the service data. For the api which has write
 *  access, you can also call the `service.set()` to store your service data.
 *
 * @note  DO NOT carry over the service data to another place, the only safe
 *         place for it is leaving it inside the service api scope
 *        API message name is end with '_req' for the request, with '_resp'
 *         for the response.
 */
static
void ranking(const skullcpp::Service& service,
             const google::protobuf::Message& request,
             google::protobuf::Message& response) {
    const auto* rankingCache = (RankingCache*)service.get();
    const auto& rankReq = (const rank_req&)request;

    int recordSz = rankReq.rrecord_size();
    if (!recordSz) {
        return;
    }

    // Prepare the ranking records
    RankingCache::RankingRecords records;

    for (int i = 0; i < recordSz; i++) {
        const auto& record = rankReq.rrecord(i);
        RankingCache::RankingRecord rankRecord;

        rankRecord.ip_    = record.ip();
        rankRecord.qtype_ = rankReq.qtype() == 1
                                ? RankingCache::QTYPE::A
                                : RankingCache::QTYPE::AAAA;
        rankRecord.expiredTime_ = record.expiredtime();
        rankRecord.score_ = 0.0f;

        records.push_back(rankRecord);
    }

    // Rank it
    rankingCache->rank(service, rankReq.question(), records);

    // Get Ranking Results
    rankingCache->rankResult(rankReq.question(), records);

    time_t now = time(NULL);
    auto& rankResp = (rank_resp&)response;

    for (const auto& result : records) {
        auto* res = rankResp.add_result();
        res->set_ip(result.ip_);
        res->set_ttl((int)(result.expiredTime_ - now));
        res->set_score(result.score_);
    }
}

/**************************** Register Service ********************************/
// Register Read APIs Here
static skullcpp::ServiceReadApi api_read_tbl[] = {
    /**
     * Format: {API_Name, API_Entry_Function}, e.g. {"get", skull_service_get}
     */
    {"rank", ranking},
    {NULL, NULL}
};

// Register Write APIs Here
static skullcpp::ServiceWriteApi api_write_tbl[] = {
    /**
     * Format: {API_Name, API_Entry_Function}, e.g. {"set", skull_service_set}
     */
    {NULL, NULL}
};

// Register Service Entries
static skullcpp::ServiceEntry service_entry = {
    skull_service_init,
    skull_service_release,
    api_read_tbl,
    api_write_tbl
};

SKULLCPP_SERVICE_REGISTER(&service_entry)
