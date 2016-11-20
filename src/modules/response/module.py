import yaml
import pprint

from skullpy import txn     as Txn
from skullpy import txndata as TxnData
from skullpy import logger  as Logger

from skull.common import protos  as Protos
from skull.common import metrics as Metrics
from skull.common.proto import *

from dnslib import *

##
# Module Init Entry, be called when start phase
#
# @param config  A parsed yamlObj
#
def module_init(config):
    Logger.debug("Response module init")
    Logger.info('0', 'config: {}'.format(pprint.pformat(config)))
    return

##
# Module Release Function, be called when shutdown phase
#
def module_release():
    Logger.debug("Response module release")
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
    #Logger.debug("response module pack")

    sharedData = txn.data()
    req      = DNSRecord.parse(sharedData.rawRequest)
    answer   = req.reply()
    question = sharedData.question

    module_counter = Metrics.module()
    domain_counter = Metrics.domain(question)

    # Assemble response
    if txn.status() != Txn.Txn.TXN_OK:
        Logger.error('ModulePack', 'Error occurred, no answer for question: {}'.format(question),
                'Please check previous errors/exceptions')

        # Increase error counter
        module_counter.error.inc(1)
        domain_counter.error.inc(1)
    else:
        # Assemble DNS response
        nanswers = len(sharedData.record)
        ips = []

        for record in sharedData.record:
            answer.add_answer(RR(question, QTYPE.A, rdata = A(record.ip), ttl=record.ttl))
            ips.append(record.ip)

        # Increase response counter
        module_counter.response.inc(1)
        domain_counter.response.inc(1)
        Logger.info('ModulePack', 'Question: {} {} Answers: {}'.format(question, nanswers, ips))

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
    #Logger.debug("response module run")
    return True
