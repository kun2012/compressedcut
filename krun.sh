#!/bin/bash
INPUT_DIR="filters"
OUTPUT_DIR="output"

for i in acl1 ipc1 fw1
do
    for j in 100 1K 5K 10K
    do
        filter="$i"_$j
        echo $filter
        rf="$INPUT_DIR"/"$filter".txt
        tf="$INPUT_DIR"/"$filter"_trace.txt
        ARGS=("-r$rf" "-w$tf")
        outputfile="$OUTPUT_DIR"/$filter.stat
        ./compressedcuts ${ARGS[@]}>$outputfile
    done
done
