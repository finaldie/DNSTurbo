import pprint

from skull import logger
from skull.txn import Txn

from common.proto import service_dns_query_req_pto


##
# Module Init Entry, be called when start phase
#
# @param config  A parsed yamlObj
#
def module_init(config):
    logger.debug("dns_ask module init")
    logger.info('ModuleInit', 'config: {}'.format(pprint.pformat(config)))
    return True


##
# Module Release Function, be called when shutdown phase
#
def module_release():
    logger.debug("dns_ask module release")
    return


##
# Module Runnable Entry, be called when this module be picked up in current
#  workflow
#
# @param txn Transaction context
#
# @return - True if no error
#         - False if error occurred
def module_run(txn: Txn):
    logger.debug("dns_ask module run")

    sharedData = txn.data()
    question = sharedData.question
    question_type = sharedData.qtype

    dns_query = service_dns_query_req_pto.query_req()
    dns_query.question = question
    dns_query.qtype = question_type == 1 and 1 or 2
    logger.debug("question: {}, qtype: {}".format(question, question_type))

    ret = txn.iocall('dns', 'query', dns_query, api_cb=_dns_response)
    if ret == Txn.IO_OK:
        return True
    else:
        logger.error(
                "DNS_E1", "Dns call failed, ret: {}".format(ret),
                'Check the parameters of service call')
        return False


def _dns_response(txn, iostatus, api_name, request_msg, response_msg):
    if iostatus != Txn.IO_OK:
        logger.error(
                "DNS_E2", "Service Busy. err code: {}.".format(iostatus),
                'Consider to increase the service queue length')
        return False

    sharedData = txn.data()

    for record in response_msg.record:
        if logger.isDebugEnabled():
            logger.debug(
                "For question: {}, qtype: {}, got ip: {}, ttl: {}".format(
                    sharedData.question, sharedData.qtype, record.ip,
                    record.ttl))

        sharedData.record.add(ip=record.ip, ttl=record.ttl)

    return True
