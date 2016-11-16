Skull Functional Test
=====================

## Folder Structure
* **common:** All common scripts should be put here
* **cases:**  Every functional case should have its own case folder here

## How to start
### Create a case folder under `cases`
```console
bash: ~/project/tests/cases>tree
.
├── basic
│   └── skull_ft_case.yml
```
### Create a skull_ft_case.yml
```console
# skull_ft_case.yml
description: Basic test case, create a default project/workflow/module/service, then verify there is no errors during creation and building

pre-run:
    - echo "pre-run"
    - ${COMMON}/skullft_create_project
    - ${COMMON}/skullft_create_workflow -c y -I example -i n -n y -p 7758
    - ${COMMON}/skullft_create_module -n test -i 0 -l cpp
    - ${COMMON}/skullft_create_service -n s1 -l cpp -m rw-pr
    - ${COMMON}/skullft_create_service_api -s s1 -n get
    - cd project && skull build && skull build check && skull build valgrind-check && skull deploy

run:
    - echo "run"
    - cd project && skull start -D
    - sleep 1

verify:
    - echo "verify"
    - echo "hello skull" | nc 127.0.0.1 7758 -i 1 | grep "hello skull"
    - cat ${RUN}/project/run/log/skull.log | grep "skull engine is ready"
    - 'cat ${RUN}/project/run/log/skull.log | grep "receive data: hello skull"'
    - 'cat ${RUN}/project/run/log/skull.log | grep "module_pack(test): data sz:12"'
    - 'cat ${RUN}/project/run/log/skull.log | grep "config test_item: 123"'
    - 'cat ${RUN}/project/run/log/skull.log | grep "config test_rate: 1"'
    - 'cat ${RUN}/project/run/log/skull.log | grep "config test_name: module"'
    - 'cat ${RUN}/project/run/log/skull.log | grep "config test_bool: 1"'
    - 'echo "metrics" | nc 127.0.0.1 7759 | grep "global.response: 1"'

post-verify:
    - echo "post-verify"
    - killall skull-engine || exit 0
```
### Then run the case
```console
skull build ft-check [case=$caseName]
```

### Basic Macros
To better writing the `skull_ft_case.yml`, some times you will need to copy some files from case folder to running folder, here are some basic macros we can use:
* **{COMMON}** It will be replaced with the `common` folder path
* **{RUN}** It will be replaced with the `running` folder path
* **{CASE}** It will be replaced with the `case` folder path
