#!/bin/bash
INPUT_DIR="filters"
OUTPUT_DIR="output"

#Hypercut -->not works fine
#binth=16
#spfac=8
#hypercuts=1
#compressionON=0
#binningON=0
#mergingON=0
#fineOn=0
#ruleMoveUp=15

#Efficuts
binth=16
spfac=8
hypercuts=1
compressionON=1
binningON=1
mergingON=1
fineOn=1
ruleMoveUp=0

mkdir -p $OUTPUT_DIR

for i in acl1 ipc1 fw1
do
    for j in 100 1K 5K 10K
    do
        filter="$i"_$j
        echo $filter
        rf="$INPUT_DIR"/"$filter".txt
        tf="$INPUT_DIR"/"$filter"_trace.txt
        ARGS=("-r$rf" "-w$tf" "-b$binth" "-s$spfac" "-m$hypercuts" "-u$ruleMoveUp" "-c$compressionON" "-g$binningON" "-z$mergingON" "-F$fineOn")
        outputfile="$OUTPUT_DIR"/$filter.stat
        ./compressedcuts ${ARGS[@]}>>$outputfile
    done
done
