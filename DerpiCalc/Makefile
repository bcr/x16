include ../x16cl65.mak

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

all: dc.prg

dc.o: dc.c ui.h
screen.o: screen.c screen.h
ui.o: ui.c ui.h screen.h

dc.prg: dc.o screen.o ui.o

clean:
	rm -f *.o
	rm -f dc.prg
