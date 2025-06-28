#!/bin/bash

DEBUG_FLAGS=""
if [ ! -z "$1" ]; then
    for arg in "$@"; do
        case $arg in
            -p)
                DEBUG_FLAGS+="DEBUG_TRACE_PARSER=1 "
                ;;
            -c)
                DEBUG_FLAGS+="DEBUG_TRACE_CODEGEN=1 "
                ;;
            -e)
                DEBUG_FLAGS+="DEBUG_TRACE_EXECUTION=1 "
                ;;
            *)
                echo "Invalid option: $arg" 1>&2
                exit 1
                ;;
        esac
    done
fi

if [ ! -z "$DEBUG_FLAGS" ]; then
    make clean
    make $DEBUG_FLAGS
else
    make
fi

COMPILER=./luac

for test_file in test/*.lua; do
    expected_file=${test_file%.lua}.expected
    output_file=${test_file%.lua}.output
    debug_log=${test_file%.lua}.log

    echo "Running test: $test_file"
    timeout 30s $COMPILER "$test_file" > "$output_file" 2> "$debug_log"

    if diff -q "$output_file" "$expected_file"; then
        echo "Test passed!"
    else
        echo "Test failed!"
        echo "Diff:"
        diff "$output_file" "$expected_file"
        echo "Output:"
        cat "$output_file"
        echo "Expected:"
        cat "$expected_file"
        if [ -s "$debug_log" ]; then
            echo "Debug log:"
            cat "$debug_log"
        fi
        exit 1
    fi
done
