# Include the basic Makefile template
include $(shell skull-config --cpp-inc)

# Implicit include .Makefile.inc from top folder if exist
-include $(SKULL_SRCTOP)/.Makefile.inc

# Including folders
INC += \
    $(shell skull-config --common-includes)

# User ldflags and dependency libraries
DEPS_LDFLAGS += \
    $(shell skull-config --common-ldflags)

DEPS_LIBS += \
    $(shell skull-config --common-libs)

TEST_DEPS_LIBS += \
    $(shell skull-config --common-test-libs)

# Source files
SRCS = $(shell find src -name "*.c*")

# Source files of UT
TEST_SRCS = $(shell find tests -name "*.cpp")

# valgrind suppresion file
SUPPRESSION +=

# Include the basic Makefile targets
include $(shell skull-config --cpp-common-targets)

# Notes: There are some available targets we can use if needed
#  prepare - This one is called before compilation
#  check   - This one is called when doing the Unit Test
#  valgrind-check - This one is called when doing the memcheck for the Unit Test
#  deploy  - This one is called when deployment be triggered
#  clean   - This one is called when cleanup be triggered

