
TARGET := all
MODULE_NAME ?= $(shell basename $(shell pwd))
CONF_TARGET ?= skull-modules-$(MODULE_NAME).yaml

MODULE_PARENT_DIR := $(DEPLOY_DIR)/lib/py/modules
MODULE_DIR := $(MODULE_PARENT_DIR)/$(MODULE_NAME)

# Include the basic Makefile targets
include $(SKULL_SRCTOP)/.skull/makefiles/Makefile.mod.py.targets

# Notes: There are some available targets we can use if needed
#  prepare - This one is called before compilation
#  check   - This one is called when doing the Unit Test
#  valgrind-check - This one is called when doing the memcheck for the Unit Test
#  deploy  - This one is called when deployment be triggered
#  clean   - This one is called when cleanup be triggered

