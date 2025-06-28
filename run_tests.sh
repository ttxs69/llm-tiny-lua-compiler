#!/bin/bash

for test_file in test/*.lua; do
    expected_file=${test_file%.lua}.expected
    output_file=${test_file%.lua}.output

    echo "Running test: $test_file"
    timeout 30s ./luac "$test_file" > "$output_file"

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
        exit 1
    fi
done
