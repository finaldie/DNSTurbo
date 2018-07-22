from skull import *

def _create_result_item(ip, ttl, latency, httpInfoCnt):
    return {
        'ip' : ip,
        'ttl': ttl,
        'avgLatency': latency,  # Integer
        'httpInfoCnt': httpInfoCnt
    }

def _item_cmp(left, right):
    return left['avgLatency'] - right['avgLatency']

# Score each record, and return a list of record with ascending order latency
def score(records):
    scoringTmp = []

    # Calculate the latency score according to the http status info
    for record in records:
        ip = record.ip
        ttl = record.ttl

        httpInfos = record.http_info
        totalLatency = 0
        validRecords = 0

        for httpInfo in httpInfos:
            # status list:
            #  0: OK
            #  1: ERROR
            #  2. TIMEOUT
            status   = httpInfo.status
            httpCode = httpInfo.httpCode
            latency  = httpInfo.latency

            if status != 1:
                totalLatency += latency
            else:
                totalLatency += 2000 # For error connection record, increase 2000ms

            validRecords += 1

        # Calculate average latency and build a scoring result
        avgLatency = 0

        if validRecords > 0:
            avgLatency = int(totalLatency / validRecords)

        scoringTmp.append(_create_result_item(ip, ttl, avgLatency, validRecords))

    # Ranking
    if logger.isDebugEnabled():
       logger.debug("scoringTmp: {}".format(scoringTmp))

    scoringResults = sorted(scoringTmp, key = lambda record: record['avgLatency'])

    if logger.isDebugEnabled():
        logger.debug("scoringResults: {}".format(scoringResults))

    return scoringResults

# Rank and filter high latency record
def rank(scoringResults, low_latency_bar, latency_factor):
    rankingResults = []

    # 1. checking total record count
    nrecords = len(scoringResults)

    if nrecords <= 1:
        return scoringResults, 0

    # 2. Keep latency <= 20ms, otherwise filter the record shouldn't larger than
    #  1.5x of the first record
    baseRecord = None
    baseRecordLatency = 0

    for scoringRecord in scoringResults:
        latency = scoringRecord['avgLatency']

        # Add record which its latency <= 20ms
        if latency <= low_latency_bar:
            rankingResults.append(scoringRecord)
            continue
        elif baseRecord is None:
            baseRecord = scoringRecord
            baseRecordLatency = baseRecord['avgLatency']

        factor = float(latency) / float(baseRecordLatency)

        if factor <= latency_factor:
            rankingResults.append(scoringRecord)
        else:
            logger.info("Ranking terminated at latency: {}, factor: {}, base: {}".format(
                latency, factor, baseRecordLatency));
            break

    nRankingRecords = len(rankingResults)

    return rankingResults, nrecords - nRankingRecords
