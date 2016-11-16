# The utility Makefile be included by other module Makefiles

debug ?= false
CXX ?= g++
strict ?= true

# Compiling flags
STD = -std=c++0x
WARN = -Wall
BASIC = -fPIC
EXTRA = -Wextra -Wno-unused-parameter -Wno-unused-function -Wfloat-equal
EXTRA += -Wdisabled-optimization
# It's better to use `-fstack-protector-strong`, but most of environment do not
# have gcc 4.9, so use `-fstack-protector` first
EXTRA += -fstack-protector -fno-omit-frame-pointer

# Enable the following line to build a strong program
#EXTRA += -pedantic -Wconversion

# Enable compiling checking the padded/inline issue
# EXTRA += -Wpadded -Winline

SHARED = -shared
MACRO = -D_POSIX_C_SOURCE=200809L
OTHER = -pipe -g -ggdb3

ifeq ($(debug), false)
OPT = -O2
else
MACRO += -DSK_DEBUG -finstrument-functions
OPT = -O0
# If your compiler is gcc(>=4.8) or clang, enable the following line
#EXTRA += -fsanitize=address
endif

ifeq ($(strict),true)
WARN += -Werror
endif

INC +=

# Linking flags
DEPS_LDFLAGS += -rdynamic

# linking flags (for unit test)
DEPS_LDFLAGS += \
    -Wl,-rpath,./lib \
    -Wl,-rpath,../../common/cpp/lib \
    -Wl,-rpath,/usr/local/lib

# Skull cc and ld
SKULL_CFLAGS = $(CFLAGS) $(CXXFLAGS) $(STD) $(WARN) $(BASIC) $(EXTRA) $(MACRO) $(OPT) $(OTHER) $(INC)
SKULL_CC = $(CXX) $(SKULL_CFLAGS)

SKULL_TP_CFLAGS = $(CFLAGS) $(CXXFLAGS) $(STD) $(WARN) $(BASIC) $(MACRO) $(OPT) $(OTHER) $(INC)
SKULL_TP_CC = $(CXX) $(SKULL_TP_CFLAGS)

SKULL_LDFLAGS = $(LDFLAGS) $(SHARED) $(OPT) $(OTHER) $(DEPS_LDFLAGS)
SKULL_LD = $(CXX) $(SKULL_LDFLAGS)

SKULL_BIN_LDFLAGS = $(LDFLAGS) $(OPT) $(OTHER) $(DEPS_LDFLAGS)
SKULL_BIN_LD = $(CXX) $(SKULL_BIN_LDFLAGS)

# Obj compiling
%.pb.o: %.pb.cc
	$(SKULL_TP_CC) -c $< -o $@

%.o: %.cpp
	$(SKULL_CC) -c $< -o $@

# Objs, user need to fill the `SRCS` and `TEST_SRCS` variable
OBJS = $(patsubst %.cc, %.o, $(patsubst %.cpp, %.o, $(SRCS)))
TEST_OBJS = $(patsubst %.cpp, %.o, $(TEST_SRCS))

# bin/lib Name
DIRNAME ?= $(shell basename $(shell pwd))
PARENT_DIRNAME ?= $(shell basename $(shell pwd | xargs dirname))
TARGET ?= lib/libskull-$(PARENT_DIRNAME)-$(DIRNAME).so
TEST_TARGET ?= test.out
CONF_TARGET ?= skull-$(PARENT_DIRNAME)-$(DIRNAME).yaml
