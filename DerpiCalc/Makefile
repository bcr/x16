include ../x16cl65.mak

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

all: dc.prg

cell.o: cell.c cell.h
dc.o: dc.c cell.h ui.h
screen.o: screen.c screen.h
ui.o: ui.c ui.h screen.h cell.h

dc.prg: dc.o cell.o screen.o ui.o

clean:
	rm -f *.o
	rm -f dc.prg
