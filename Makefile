CC = cl65
TARGET_ARCH = -t cx16

# Cribbed from
# %: %.o
#  commands to execute (built-in):
#        $(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

all: charmap.prg boxtext.prg

charmap.prg: charmap.o

boxtext.prg: boxtext.o
