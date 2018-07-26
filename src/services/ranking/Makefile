# Include the basic Makefile template
include $(shell skull-config --cpp-inc)

# Implicit include .Makefile.inc from top folder if exist
-include $(SKULL_SRCTOP)/.Makefile.inc

INC += \
    -I$(SKULL_SRCTOP)/deps/http-parser \
    $(shell skull-config --includes)

DEPS_LDFLAGS += \
    -L$(SKULL_SRCTOP)/deps/http-parser \
    $(shell skull-config --ldflags)

DEPS_LIBS += \
    -lhttp_parser \
    $(shell skull-config --libs)

TEST_DEPS_LIBS += \
    -lhttp_parser \
    $(shell skull-config --test-libs)

# Objs and deployment related items
SRCS = \
    src/service.cpp \
    src/RankingCache.cpp \
    src/HttpRequest.cpp \
    src/HttpResponseImp.cpp \
    src/EPHandler.cpp

TEST_SRCS = \
    tests/test_service.cpp

# valgrind suppresion file
SUPPRESSION +=

# Include the basic Makefile targets
include $(shell skull-config --cpp-targets)

# Notes: There are some available targets we can use if needed
#  prepare - This one is called before compilation
#  check   - This one is called when doing the Unit Test
#  valgrind-check - This one is called when doing the memcheck for the Unit Test
#  deploy  - This one is called when deployment be triggered
#  clean   - This one is called when cleanup be triggered

build_http_parser:
	cd $(SKULL_SRCTOP)/deps/http-parser && make package CFLAGS_FAST_EXTRA+=-fPIC

clean_http_parser:
	cd $(SKULL_SRCTOP)/deps/http-parser && make clean

prepare: build_http_parser

clean: clean_http_parser

