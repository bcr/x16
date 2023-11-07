include ../x16cl65.mak

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

all: dc.prg

cell.o: cell.c cell.h
dc.o: dc.c cell.h input.h
dc_math.o: dc_math.c dc_math.h
input.o: input.c input.h keyboard.h ui.h
keyboard.o: keyboard.c keyboard.h
screen.o: screen.c screen.h
ui.o: ui.c ui.h ui_internal.h screen.h cell.h
ui_edit.o: ui_edit.c ui.h ui_internal.h

dc.prg: dc.o dc_math.o cell.o input.o keyboard.o screen.o ui.o ui_edit.o

docs:
	doxygen

clean:
	rm -f *.o
	rm -f dc.prg
