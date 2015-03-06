#!/bin/bash
N=330

# # Base HiCuts
# COM="hicut"
# M=0
# C=0
# G=0
# Z=0
# F=0

# Plus HyperCuts
COM="hyper"
M=1
C=0
G=0
Z=0
F=0

# Plus EffiCuts
# COM="effi"
# M=1
# C=1
# G=1
# Z=1
# F=0

# #FineCuts
# COM="fine"
# M=0
# C=1
# G=1
# Z=1
# F=1

binth=8

#TARGET="tests_color_nl/n$N"
#TARGET="rl_rr_nl"
#TARGET="dalyopenflow"
#TARGET="partition/1sets/n330"
#TARGET="allof"
# TARGET="synth_1k_nl"
K=4
R=4
TARGET="wide_synth/${K}k_${R}"
INPUT_DIR="classifiers/$TARGET"
OUTPUT_DIR="output/$TARGET/$COM$binth"

FILES=$INPUT_DIR/*

mkdir -p $OUTPUT_DIR

for file in $FILES
do
	filename=$(basename $file)
	extension=${filename##*.}
	filename=${filename%.*}
	echo $filename
	OUTPUT_FILE="${OUTPUT_DIR}/${filename}.txt"
	ARGS=("-r" $file "-m$M" "-c$C" "-g$G" "-z$Z" "-F$F" "-b$binth" "-R${R}")

	time ./compressedcuts ${ARGS[@]} > $OUTPUT_FILE
done

echo "done"
exit