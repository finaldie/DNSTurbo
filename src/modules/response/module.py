import yaml
import pprint
import time

from skullpy import *
from skullpy.txn import *

from skull.common import *
from skull.common.proto import *

from dnslib import *

##
# Module Init Entry, be called when start phase
#
# @param config  A parsed yamlObj
#
def module_init(config):
    logger.debug("Response module init")
    logger.info('0', 'config: {}'.format(pprint.pformat(config)))
    return

##
# Module Release Function, be called when shutdown phase
#
def module_release():
    logger.debug("Response module release")
    return

##
# Input data unpack function, be called if this module is the 'last' module in
#  the workflow (It would no effect if there is no response needed)
#
# @param txn  Transaction context which is used for getting shared transaction
#              data or invoking service `iocall`
# @param data Input data
#
# @return How many bytes be consumed
#
def module_pack(txn, txndata):
    #logger.debug("response module pack")

    sharedData = txn.data()
    req      = DNSRecord.parse(sharedData.rawRequest)
    answer   = req.reply()
    question = sharedData.question
    qtype    = QTYPE[req.q.qtype]

    module_counter = metrics.module()
    qtype_counter  = metrics.qtype(qtype)

    # Assemble response
    if txn.status() != Txn.TXN_OK:
        logger.error('ModulePack', 'Error occurred, no answer for question: {}, type: {}'.format(question, qtype),
                'Please check previous errors/exceptions')

        # Increase error counter
        module_counter.error.inc(1)
        qtype_counter.error.inc(1)
    else:
        # Assemble DNS response
        nRawRecords = len(sharedData.record)
        nAnswers    = len(sharedData.rankingRecord)
        nFiltered   = nRawRecords - nAnswers
        ips = []

        for record in sharedData.rankingRecord:
            if req.q.qtype == QTYPE.A:
                answer.add_answer(RR(question, req.q.qtype, rdata = A(record.ip), ttl=record.ttl))
            elif req.q.qtype == QTYPE.AAAA:
                answer.add_answer(RR(question, req.q.qtype, rdata = AAAA(record.ip), ttl=record.ttl))

            ips.append((record.ip, record.ttl))

        # Increase response counter
        module_counter.response.inc(1)
        module_counter.total_records.inc(nAnswers)
        module_counter.total_filtered.inc(nFiltered)

        qtype_counter.response.inc(1)
        qtype_counter.total_records.inc(nAnswers)
        qtype_counter.total_filtered.inc(nFiltered)

        duration = (time.time() - sharedData.startTime) * 1000
        logger.info('ModulePack', 'Duration: {} ,filtered: {} ,Question: {} ,type: {}, {} Answers: {}'.format(
            duration, nFiltered, question, qtype, nAnswers, ips))

    txndata.append(answer.pack())

##
# Module Runnable Entry, be called when this module be picked up in current
#  workflow
#
# @param txn Transaction context
#
# @return - True if no error
#         - False if error occurred
def module_run(txn):
    #logger.debug("response module run")
    return True

