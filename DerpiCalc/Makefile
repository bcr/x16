include ../x16cl65.mak

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

all: dc.prg

cell.o: cell.c cell.h
dc.o: dc.c cell.h input.h
input.o: input.c input.h keyboard.h ui.h
keyboard.o: keyboard.c keyboard.h
screen.o: screen.c screen.h
ui.o: ui.c ui.h screen.h cell.h

dc.prg: dc.o cell.o input.o keyboard.o screen.o ui.o

docs:
	doxygen

clean:
	rm -f *.o
	rm -f dc.prg
