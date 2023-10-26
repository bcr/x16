all: charmap.prg boxtext.prg

charmap.prg: charmap.c
	cl65 -o charmap.prg -t cx16 charmap.c

boxtext.prg: boxtext.c
	cl65 -o boxtext.prg -t cx16 boxtext.c
