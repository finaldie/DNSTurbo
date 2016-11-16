# Include the basic Makefile template
include $(SKULL_SRCTOP)/.skull/makefiles/Makefile.cpp.inc

# Include folders
INC +=

# User ldflags and dependency libraries
DEPS_LDFLAGS +=
DEPS_LIBS += \
    -lskullcpp-api \
    -Wl,--no-as-needed \
    -lprotobuf

TEST_DEPS_LIBS += \
    -lskull-unittest-cpp \
    -lskull-unittest-c \
    -lprotobuf

# Source files
SRCS = $(shell find src -name "*.c*")

# Source files of UT
TEST_SRCS = $(shell find tests -name "*.cpp")

# valgrind suppresion file
#  note: if the suppresion file is exist, then need to append
#        `--suppresions=$(SUPPRESION)` to `VALGRIND`
SUPPRESION :=

# valgrind command
VALGRIND := valgrind --tool=memcheck --leak-check=full \
    --gen-suppressions=all --error-exitcode=1

# Include the basic Makefile targets
include $(SKULL_SRCTOP)/.skull/makefiles/Makefile.common.cpp.targets

# Notes: There are some available targets we can use if needed
#  prepare - This one is called before compilation
#  check   - This one is called when doing the Unit Test
#  valgrind-check - This one is called when doing the memcheck for the Unit Test
#  deploy  - This one is called when deployment be triggered
#  clean   - This one is called when cleanup be triggered

