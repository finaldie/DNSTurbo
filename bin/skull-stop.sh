#!/bin/sh

if [ $# = 0 ]; then
    echo "Fatal: Missing the pid of skull"
    exit 1
fi

# 1. Put some settings/actions here...

# 2. Stop the skull engine
skull_pid=$1

kill $skull_pid
