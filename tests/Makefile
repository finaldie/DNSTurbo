OUTPUT_FOLDER = ./run
EXTRA_ARGS =
EXTRA_TITLE =
CASE_NAME =

debug = false
case =
skip =

ifneq ($(case),)
    EXTRA_ARGS += -n $(case)
endif

ifneq ($(skip),)
    EXTRA_ARGS += --skip $(skip)
endif

ifeq ($(debug), true)
    EXTRA_ARGS += -D
    EXTRA_TITLE = "(Debug Mode)"
endif

check: prepare clean
	@echo "Skull FT Cases Running ..." $(EXTRA_TITLE)
	@skull-ft -p ./ -l 30 $(EXTRA_ARGS)

prepare:
	@mkdir -p $(OUTPUT_FOLDER)

clean:

help:
	@echo "usage:"
	@echo " - make [case=CaseName] [skip=case1[,case2...]] [debug=true]"
