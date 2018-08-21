#!/bin/bash

set -e
ulimit -c unlimited

##################################### Utils ####################################
setup_env() {
    skull_bin=`which skull-engine`
    if [ $? -ne 0 ]; then
        echo "Error: Not found skull-engine, install it first. Abort" >&2
        exit 1
    fi
}

usage() {
    echo "usage:"
    echo "  skull-start.sh -c config [-D|--memcheck|--gdb|--strace|--massif|--no-log-rolling|--std-fowarding]"
}

skull_start() {
    local args=""
    if $no_log_rolling; then
        args="$args -n"
    fi

    if $std_forwarding; then
        args="$args -s"
    fi

    if ! $daemon; then
        exec $skull_bin -c $skull_config $args
    else
        exec $skull_bin -c $skull_config $args -D > log/stdout.log 2>&1 < /dev/null
    fi
}

skull_start_memcheck() {
    # Generate suppression argument
    local sys_path=`skull-config --valgrind-dir`
    local local_path="bin"

    local supp_files=`ls ${sys_path}/*.supp`
    supp_file+=`find ${local_path} -name "*.supp"`
    local supp_arg=""

    for supp_file in $supp_files; do
        supp_arg="$supp_arg --suppressions="${supp_file}" "
    done

    # Run valgrind to start skull
    exec valgrind --tool=memcheck --leak-check=full --num-callers=50 -v \
        ${supp_arg} \
        $skull_bin -c $skull_config
}

skull_start_gdb() {
    echo "After gdb started, type 'run -c $skull_config'"
    echo ""
    exec gdb $skull_bin
}

skull_start_strace() {
    exec strace -f $skull_bin -c $skull_config
}

skull_start_massif() {
    echo "After profiling, run 'ms_print massif.out.pid' to analyze the result"
    echo ""
    exec valgrind --tool=massif $skull_bin -c $skull_config
}
################################## End of Utils ################################

if [ $# = 0 ]; then
    echo "Error: Missing parameters" >&2
    usage >&2
    exit 1
fi

# 1. Parse the parameters
skull_config=""
memcheck=false
run_by_gdb=false
run_by_strace=false
massif=false
daemon=false
no_log_rolling=false
std_forwarding=false
skull_bin=

setup_env

args=`getopt -a \
        -o c:Dnh \
        -l memcheck,gdb,strace,massif,no-log-rolling,std-forwarding,help \
        -n "skull-start.sh" -- "$@"`
if [ $? != 0 ]; then
    echo "Error: Invalid parameters" >&2
    usage >&2
    exit 1
fi

eval set -- "$args"

while true; do
    case "$1" in
        -c)
            shift
            skull_config=$1
            shift
            ;;
        -D)
            shift
            daemon=true
            ;;
        -n|--no-log-rolling)
            shift
            no_log_rolling=true
            ;;
        --std-forwarding)
            shift
            std_forwarding=true
            ;;
        --memcheck)
            shift
            memcheck=true
            ;;
        --gdb)
            shift
            run_by_gdb=true
            ;;
        --strace)
            shift
            run_by_strace=true
            ;;
        --massif)
            shift
            massif=true
            ;;
        -h|--help)
            shift
            usage >&2
            exit 0
            ;;
        --)
            shift; break
            ;;
        (*)
            echo "Error: Unknown parameter $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

# 2. Check parameters
if [ -z "$skull_config" ]; then
    echo "Fatal: Missing skull parameters" >&2
    usage >&2
    exit 1
fi

# 3. Rrepare the environment variables
skull_rundir=`dirname $skull_config`
export LD_LIBRARY_PATH=$skull_rundir/lib

# 4. Put some custom settings/actions here...

# 5. Start skull
if $memcheck; then
    skull_start_memcheck
elif $run_by_gdb; then
    skull_start_gdb
elif $run_by_strace; then
    skull_start_strace
elif $massif; then
    skull_start_massif
else
    skull_start
fi
