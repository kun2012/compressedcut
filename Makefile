CPP = g++
CFLGA = -lm -o3 -std=c++11

compressedcuts: compressedcuts.c checks.c cutio.c shared.c fine.c merging.c compressedcuts.h shared.h tracehd.c
	${CPP} ${CFLGA} -o compressedcuts compressedcuts.c checks.c cutio.c shared.c fine.c merging.c tracehd.c

all: compressedcuts

clean: 
	rm -f compressedcuts
