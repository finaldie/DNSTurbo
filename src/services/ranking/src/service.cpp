#include <stdlib.h>

#include <iostream>
#include <string>
#include <google/protobuf/message.h>

#include <skullcpp/api.h>
#include "skull_protos.h"
#include "config.h"

#include "RankingCache.h"

#define RANKING_CACHE_RANK_INTERVAL    (10 * 1000)
#define RANKING_CACHE_STATUS_INTERVAL  (5 * 1000)

using namespace skull::service::ranking;

/**************************** Internal APIs ***********************************/
static void _rankingCacheCleanup(skullcpp::Service& service);

static
void _rankingRecordUpdating(skullcpp::Service& service,
                            const std::string& question,
                            const RankingCache::RankingRecords& records) {
    auto* cache = (RankingCache*)service.get();
    cache->addIntoCache(question, records);
}

static
void _cleanupError(const skullcpp::Service& service) {
    SKULLCPP_LOG_ERROR("_cleanupError", "_cleanup error occurred",
        "Reduce the service load or increase the service task queue size");

    const auto& config = skullcpp::Config::instance();
    int ret = service.createJob((uint32_t)config.cleanup_interval(), 1,
                      skull_BindSvcJobNPW(_rankingCacheCleanup), _cleanupError);
    if (ret) {
        SKULLCPP_LOG_ERROR("_cleanupError", "Create job failed", "Check the parameters");
    }
}

static
void _rankingCacheCleanup(skullcpp::Service& service) {
    const auto& config = skullcpp::Config::instance();
    auto* cache = (RankingCache*)service.get();
    cache->cleanup(config.cleanup_delayed());

    // Set up a clean up job
    int ret = service.createJob((uint32_t)config.cleanup_interval(), 1,
                      skull_BindSvcJobNPW(_rankingCacheCleanup), _cleanupError);
    if (ret) {
        SKULLCPP_LOG_ERROR("Cleanup", "Create job failed", "Check the parameters");
    }
}

static
void _rankingCacheSpeedTest(const skullcpp::Service& service) {
    const auto* cache = (const RankingCache*)service.get();
    cache->doSpeedTest(service);

    // Set up a ranking job
    service.createJob(RANKING_CACHE_RANK_INTERVAL, 1,
                      skull_BindSvcJobNPR(_rankingCacheSpeedTest), NULL);
}

static
void _rankingCacheStatus(const skullcpp::Service& service) {
    const auto* cache = (const RankingCache*)service.get();
    const std::string statusStr = cache->status();

    SKULLCPP_LOG_INFO("cache status", statusStr);

    // Set up a ranking job
    service.createJob(RANKING_CACHE_STATUS_INTERVAL, 1,
                      skull_BindSvcJobNPR(_rankingCacheStatus), NULL);
}

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

    // Set up a clean up job
    const auto& conf = skullcpp::Config::instance();

    service.createJob((uint32_t)conf.cleanup_interval(), 1,
                      skull_BindSvcJobNPW(_rankingCacheCleanup), NULL);

    // Set up a ranking job
    service.createJob(RANKING_CACHE_RANK_INTERVAL, 1,
                      skull_BindSvcJobNPR(_rankingCacheSpeedTest), NULL);

    // Set up a ranking status job
    service.createJob(RANKING_CACHE_STATUS_INTERVAL, 1,
                      skull_BindSvcJobNPR(_rankingCacheStatus), NULL);
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
    RankingCache::QTYPE qtype = rankReq.qtype() == 1
                                ? RankingCache::QTYPE::A
                                : RankingCache::QTYPE::AAAA;

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
        rankRecord.qtype_ = qtype;
        rankRecord.expiredTime_ = record.expiredtime();

        records.push_back(rankRecord);
    }

    // Rank it via a service job
    service.createJob(0, 1, skull_BindSvcJobNPW(_rankingRecordUpdating, rankReq.question(), records), NULL);

    // Get Ranking Results
    rankingCache->rankResult(rankReq.question(), records);

    time_t now = time(NULL);
    auto& rankResp = (rank_resp&)response;

    for (const auto& result : records) {
        auto* res = rankResp.add_result();
        res->set_ip(result.ip_);
        res->set_ttl((int)(result.expiredTime_ - now));

        for (const auto& httpInfo : result.httpInfo_) {
            auto* info = res->add_http_info();
            info->set_status(httpInfo.status_);
            info->set_httpcode(httpInfo.httpCode_);
            info->set_latency(httpInfo.latency_);
        }
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
