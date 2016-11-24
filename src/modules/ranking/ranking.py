from skullpy import *

def _create_result_item(ip, ttl, latency):
    return {
        'ip' : ip,
        'ttl': ttl,
        'avgLatency': latency  # Integer
    }

def _item_cmp(left, right):
    return left['avgLatency'] - right['avgLatency']

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
                validRecords += 1

        avgLatency = 0

        if validRecords > 0:
            avgLatency = int(totalLatency / validRecords)

        scoringTmp.append(_create_result_item(ip, ttl, avgLatency))

    # Ranking
    if logger.isDebugEnabled():
       logger.debug("scoringTmp: {}".format(scoringTmp))

    scoringResults = sorted(scoringTmp, _item_cmp)

    if logger.isDebugEnabled():
        logger.debug("scoringResults: {}".format(scoringResults))

    return scoringResults

def rank(scoringResults):
    rankingResults = []

    # 1. checking total record coount
    nrecords = len(scoringResults)

    if nrecords <= 1:
        return scoringResults

    # 2. Keep latency <= 20ms, otherwise filter the record shouldn't larger than
    #  1.5x of the first record
    baseRecord = None
    baseRecordLatency = 0

    for scoringRecord in scoringResults:
        latency = scoringRecord['avgLatency']

        # Add record which its latency <= 20ms
        if latency <= 20:
            rankingResults.append(scoringRecord)
            continue
        elif baseRecord is None:
            baseRecord = scoringRecord
            baseRecordLatency = baseRecord['avgLatency']

        factor = float(latency) / float(baseRecordLatency)

        if factor <= 1.5:
            rankingResults.append(scoringRecord)
        else:
            break

    nRankingRecords = len(rankingResults)
    logger.info("Ranking", "Results: {} ; {} be filtered".format(
        rankingResults, nrecords - nRankingRecords))

    return rankingResults
