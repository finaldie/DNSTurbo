#include <stdlib.h>

#include <iostream>
#include <string>
#include <google/protobuf/message.h>

#include <skullcpp/api.h>
#include "skull_protos.h"
#include "config.h"

#include "RankingCache.h"

#define SERVICE_CRON_INTERVAL (1000)

using namespace skull::service::ranking;

static int gRound = 0;
static int gLatencyRound = 0;
static size_t gLatencyProcessed = 0;

/**************************** Internal APIs ***********************************/
static void _rankingCacheCleanup(skullcpp::Service& service);
static void _cronjob(const skullcpp::Service& service);

static
void _cleanupError(const skullcpp::Service& service) {
    SKULLCPP_LOG_ERROR("_cleanupError", "_cleanup error occurred",
        "Reduce the service load or increase the service task queue size");

    service.createJob(SERVICE_CRON_INTERVAL, 1,
                      skull_BindSvcJobNPW(_rankingCacheCleanup), _cleanupError);
}

static
void _cronjobError(const skullcpp::Service& service) {
    SKULLCPP_LOG_ERROR("_cronjobError", "_cronjob error occurred",
        "Reduce the service load or increase the service task queue size");

    service.createJob(SERVICE_CRON_INTERVAL, 1,
                      skull_BindSvcJobNPW(_cronjob), _cronjobError);
}

static
void _rankingCacheCleanup(skullcpp::Service& service) {
    SKULLCPP_LOG_DEBUG("Cleanup start...");
    const auto& config = skullcpp::Config::instance();
    auto* cache = (RankingCache*)service.get();

    // 1. Cleanup expired records
    cache->cleanup(config.cleanup_delayed());

    // 2. Ship latency test results into ranking cache
    cache->shipLatencyResults();

    // 3. Rebuild latencyCache
    cache->rebuildLatencyCache();
    gLatencyProcessed = 0;
}

static
void _cronjob(const skullcpp::Service& service) {
    const auto* cache = (const RankingCache*)service.get();
    const auto& conf = skullcpp::Config::instance();
    gRound++;

    // Latency Test
    int totalLatencyRound = conf.latency_detection_round();
    if (gRound % conf.latency_detection_interval() == 0) {
        int currRound  = gLatencyRound % totalLatencyRound; // [0, 4]
        int leftRound  = totalLatencyRound - currRound;     // [1, 4]

        size_t totalSz  = cache->latencyCacheSize();
        size_t count    = (totalSz - gLatencyProcessed) / (size_t)leftRound;
        size_t startIdx = gLatencyProcessed;
        size_t endIdx   = gLatencyProcessed + count - 1;
        SKULLCPP_LOG_INFO("LatencyTest", "TotalRound: " << totalLatencyRound
                           << " ,currRound: " << currRound
                           << " ,leftRound: " << leftRound
                           << " ,processed: " << gLatencyProcessed
                           << " ,totalSz: "   << totalSz
                           << " ,count: "     << count
                           << " ,startIdx: "  << startIdx
                           << " ,endIdx: "    << endIdx);

        cache->doSpeedTest(service, startIdx, endIdx);

        gLatencyRound++;
        gLatencyProcessed += count;

        if (gLatencyRound % totalLatencyRound  == 0) {
            service.createJob((uint32_t)conf.latency_detection_interval() * 1000, 1,
                    skull_BindSvcJobNPW(_rankingCacheCleanup), _cleanupError);
        }
    }

    // Status
    if (gRound % conf.status_interval() == 0) {
        const std::string statusStr = cache->status();
        SKULLCPP_LOG_INFO("CacheStatus", statusStr);
    }

    // Continue cronjob
    service.createJob(SERVICE_CRON_INTERVAL, 1, skull_BindSvcJobNPR(_cronjob), _cronjobError);
}

// ====================== Service Init/Release =================================
/**
 * Service Initialization. It will be called before module initialization.
 */
static
int  skull_service_init(skullcpp::Service& service, const skull_config_t* config)
{
    SKULLCPP_LOG_INFO("svc.ranking", "Skull service initializing");

    // Load skull_config to skullcpp::Config
    skullcpp::Config::instance().load(config);

    // Set Ranking Cache
    service.set(new RankingCache());

    // Set up service cron job
    service.createJob(SERVICE_CRON_INTERVAL, 1,
                      skull_BindSvcJobNPR(_cronjob), NULL);
    return 0;
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
    const std::string& question = rankReq.question();
    skullcpp::Service::JobNPW updateFunc = [question, records] (skullcpp::Service& service) {
        auto* cache = (RankingCache*)service.get();
        cache->addIntoCache(service, question, records);
    };

    service.createJob((uint32_t)0, 1, updateFunc, NULL);

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
