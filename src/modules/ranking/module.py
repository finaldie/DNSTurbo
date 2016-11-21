import yaml
import pprint
import time

from skullpy import txn     as Txn
from skullpy import txndata as TxnData
from skullpy import logger  as Logger

from skull.common import protos  as Protos
from skull.common import metrics as Metrics
from skull.common.proto import *

##
# Module Init Entry, be called when start phase
#
# @param config  A parsed yamlObj
#
def module_init(config):
    Logger.debug("py module init")
    Logger.info('ModuleInit', 'config: {}'.format(pprint.pformat(config)))
    return

##
# Module Release Function, be called when shutdown phase
#
def module_release():
    Logger.debug("py module release")
    return

##
# Module Runnable Entry, be called when this module be picked up in current
#  workflow
#
# @param txn Transaction context
#
# @return - True if no error
#         - False if error occurred
def module_run(txn):
    sharedData = txn.data()
    question = sharedData.question

    rank_query = service_ranking_rank_req_pto.rank_req()
    rank_query.question = question
    rank_query.qtype = 1

    for record in sharedData.record:
        rank_query.rRecord.add(ip = record.ip, expiredTime = int(time.time()) + record.ttl)

    ret = txn.iocall('ranking', 'rank', rank_query, 0, _ranking_response)

    return True

def _ranking_response(txn, iostatus, api_name, request_msg, response_msg):
    sharedData = txn.data()

    for record in response_msg.result:
        print "record: {}".format(record)

    return True
