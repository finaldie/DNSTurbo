# Include the basic Makefile template
include $(shell skull-config --py-inc)

# Implicit include .Makefile.inc from top folder if exist
-include $(SKULL_SRCTOP)/.Makefile.inc

# Include the basic Makefile targets
include $(shell skull-config --py-common-targets)

# Notes: There are some available targets we can use if needed
#  prepare - This one is called before compilation
#  check   - This one is called when doing the Unit Test
#  valgrind-check - This one is called when doing the memcheck for the Unit Test
#  deploy  - This one is called when deployment be triggered
#  clean   - This one is called when cleanup be triggered

