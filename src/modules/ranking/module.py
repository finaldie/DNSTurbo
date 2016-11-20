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
    Logger.debug("py module run")
    return True
