import yaml
import pprint

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
    Logger.debug("dns_ask module init")
    Logger.info('ModuleInit', 'config: {}'.format(pprint.pformat(config)))
    return

##
# Module Release Function, be called when shutdown phase
#
def module_release():
    Logger.debug("dns_ask module release")
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
    Logger.debug("dns_ask module run")

    sharedData = txn.data()
    question = sharedData.question

    dns_query = service_dns_query_req_pto.query_req()
    dns_query.question = question

    ret = txn.iocall('dns', 'query', dns_query, 0, _dns_response)
    return True

def _dns_response(txn, iostatus, api_name, request_msg, response_msg):
    sharedData = txn.data()

    for record in response_msg.record:
        Logger.debug("got ip: {}, ttl: {}".format(record.ip, record.ttl))
        sharedData.record.add(ip = record.ip, ttl = record.ttl)

    return True
