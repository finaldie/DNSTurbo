MAKE_FLAGS ?= -s
MAKE ?= make
MAKE += $(MAKE_FLAGS)
DEP_FOLDER = ./deps/skull

# global variables
export SKULL_SRCTOP := $(shell pwd)
export python_path ?= /usr/bin/python3

# Static variables
SKULL_BIN_DIR = bin
SKULL_CONFIG_DIR = config

DEPLOY_DIR_ROOT ?= $(SKULL_SRCTOP)/run
DEPLOY_BIN_ROOT := $(DEPLOY_DIR_ROOT)/bin
DEPLOY_LIB_ROOT := $(DEPLOY_DIR_ROOT)/lib
DEPLOY_LOG_ROOT := $(DEPLOY_DIR_ROOT)/log
DEPLOY_ETC_ROOT := $(DEPLOY_DIR_ROOT)/etc

# Get all the sub dirs which have Makefile
COMMON-PY  := $(shell test -d src/common-py  && find src/common-py  -maxdepth 2 -name Makefile)
COMMON-CPP := $(shell test -d src/common-cpp && find src/common-cpp -maxdepth 1 -name Makefile)
MODS := $(shell find src/modules -maxdepth 2 -name Makefile)
SRVS := $(shell find src/services -maxdepth 2 -name Makefile)

SUBS = \
    $(COMMON-PY) \
    $(COMMON-CPP) \
    $(MODS) \
    $(SRVS)

# Required by skull
build:
	@set -e; for sub in $(SUBS); do \
	    subdir=`dirname $$sub`; \
	    echo -n " - Building $$subdir ... "; \
	    $(MAKE) -C $$subdir; \
	    echo "done"; \
	done

# Required by skull
check:
	@set -e; for sub in $(SUBS); do \
	    subdir=`dirname $$sub`; \
	    echo " - Testing $$subdir ... "; \
	    $(MAKE) -C $$subdir check; \
	    echo " - Test $$subdir done"; \
	    echo ""; \
	done

# Required by skull, Only C/C++ language module need to implement it
valgrind-check:
	@set -e; for sub in $(SUBS); do \
	    subdir=`dirname $$sub`; \
	    echo " - Testing $$subdir ... "; \
	    $(MAKE) -C $$subdir valgrind-check; \
	    echo " - Test $$subdir done"; \
	    echo ""; \
	done

# Required by skull, to start the functional test
ft-check:
	@cd tests && $(MAKE)

# Required by skull
clean:
	@set -e; for sub in $(SUBS); do \
	    subdir=`dirname $$sub`; \
	    echo -n " - Cleaning $$subdir ... "; \
	    $(MAKE) -C $$subdir clean; \
	    echo "done"; \
	done
	@echo "Project clean done"

# Required by skull
deploy: prepare_deploy
	@set -e; for sub in $(SUBS); do \
	    subdir=`dirname $$sub`; \
	    echo -n " - Deploying $$subdir ... "; \
	    $(MAKE) -C $$subdir deploy DEPLOY_DIR=$(DEPLOY_DIR_ROOT); \
	    echo "done"; \
	done

dep:
	$(MAKE) -C $(DEP_FOLDER) dep

install-dep:
	$(MAKE) -C $(DEP_FOLDER) install-dep

clean-dep:
	$(MAKE) -C $(DEP_FOLDER) clean-dep

skull:
	$(MAKE) -C $(DEP_FOLDER)

install-skull:
	$(MAKE) -C $(DEP_FOLDER) install

clean-skull:
	$(MAKE) -C $(DEP_FOLDER) clean

# skull utils' targets
prepare_deploy: prepare_deploy_dirs prepare_deploy_files

prepare_deploy_dirs:
	@echo -n "Creating running environment ... "
	@test -d $(DEPLOY_DIR_ROOT) || mkdir -p $(DEPLOY_DIR_ROOT)
	@test -d $(DEPLOY_BIN_ROOT) || mkdir -p $(DEPLOY_BIN_ROOT)
	@test -d $(DEPLOY_LIB_ROOT) || mkdir -p $(DEPLOY_LIB_ROOT)
	@test -d $(DEPLOY_ETC_ROOT) || mkdir -p $(DEPLOY_ETC_ROOT)
	@test -d $(DEPLOY_LOG_ROOT) || mkdir -p $(DEPLOY_LOG_ROOT)
	@echo "done"

prepare_deploy_files:
	@echo -n "Copying basic files ... "
	@cp ChangeLog.md README.md $(DEPLOY_DIR_ROOT)
	@cp $(SKULL_CONFIG_DIR)/skull-config.yaml $(DEPLOY_DIR_ROOT)
	@cp -r $(SKULL_BIN_DIR)/* $(DEPLOY_BIN_ROOT)
	@echo "done"

.PHONY: build check valgrind-check ft-check deploy clean prepare_deploy
.PHONY: prepare_deploy_dirs prepare_deploy_files help dep install-dep
.PHONY: clean-dep skull install-skull clean-skull

help:
	@echo "make options:"
	@echo "- check"
	@echo "- valgrind-check"
	@echo "- ft-check"
	@echo "- clean"
	@echo "- deploy"
