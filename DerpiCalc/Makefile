include ../x16cl65.mak
# include ../llvm.mak

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

CFLAGS=-DDC_MATH_CX16
all: dc.prg

cell.o: cell.c cell.h dc_math.h expr.h util.h
dc.o: dc.c cell.h input.h
dc_math_cx16.o: dc_math_cx16.c dc_math.h dc_math_cx16.h
expr.o: expr.c expr.h cell.h dc_math.h util.h
expr_at.o: expr_at.c expr.h cell.h dc_math.h expr_internal.h util.h
input.o: input.c input.h keyboard.h ui.h util.h
keyboard.o: keyboard.c keyboard.h
screen.o: screen.c screen.h
ui.o: ui.c ui.h ui_internal.h screen.h cell.h util.h
ui_edit.o: ui_edit.c ui.h ui_internal.h util.h
util.o: util.c util.h

dc.prg: dc.o dc_math_cx16.o cell.o expr.o expr_at.o input.o keyboard.o screen.o ui.o ui_edit.o util.o

docs:
	doxygen

clean:
	rm -f *.o
	rm -f dc.prg
