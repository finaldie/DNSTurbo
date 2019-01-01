import pprint
import time

from dnslib import DNSRecord
from dnslib import DNSHeader
from dnslib import QTYPE
from skull import logger

from common import metrics


##
# Module Init Entry, be called when start phase
#
# @param config  A parsed yamlObj
#
def module_init(config):
    logger.debug("py module init")
    logger.info('0', 'config: {}'.format(pprint.pformat(config)))
    return True


##
# Module Release Function, be called when shutdown phase
#
def module_release():
    logger.debug("py module release")
    return


##
# Input data unpack function, be called if this module is the 'first' module in
#  the workflow and there is input data incoming
#
# @param txn  Transaction context which is used for getting shared transaction
#              data or invoking service `iocall`
# @param data Input data
#
# @return - > 0: How many bytes consumed
#         - = 0: Need more data
#         - < 0: Error occurred
#
def module_unpack(txn, data):
    # logger.debug("request module unpack")

    # Parse dns request
    request = DNSRecord.parse(data)
    question = str(request.q.qname)
    question_type = request.q.qtype
    requestId = request.header.id

    rawRecord = DNSRecord(
        DNSHeader(id=requestId, qr=1, aa=1, ra=1), q=request.q)

    # logger.debug("question: {}, type: {}, {}".format(
    #    question, question_type, QTYPE[question_type]))

    # Store data into txn sharedData
    sharedData = txn.data()
    sharedData.question = question
    if QTYPE[question_type] == 'A':
        sharedData.qtype = 1
    elif QTYPE[question_type] == 'AAAA':
        sharedData.qtype = 2
    else:
        sharedData.qtype = 0  # Error type

    sharedData.requestId = requestId
    sharedData.rawRequest = bytes(rawRecord.pack())
    sharedData.startTime = time.time()

    # Increase counters
    module_counter = metrics.module()
    qtype_counter = metrics.qtype(QTYPE[question_type])

    module_counter.request.inc(1)
    qtype_counter.request.inc(1)
    return len(data)


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

    question_type = sharedData.qtype
    if question_type != 1 and question_type != 2:
        # Currently we only support query A and AAAA record,
        #  otherwise make a failure
        return False

    return True
