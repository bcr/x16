# Commander X16 Explorations

## C Code

### Makefile Shenanigans

I have included a `Makefile` that may be instructive:

```make
CC = cl65
TARGET_ARCH = -t cx16

# Cribbed from
# %: %.o
#  commands to execute (built-in):
#        $(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.prg : %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
```

The important things are setting the `CC` and `TARGET_ARCH` and adding a
pattern rule to build `.prg` files. Other than this, everything should behave
the normal way `make` does things. The rest of this section is a review of
how `make` works.

If you have a single source file `test.c` and you add:

```make
all: test.prg
```
It will build that source file into `test.prg`

You can do something more complicated like:

```make
all: test.prg

test.o: helper.h

test.prg: test.o helper.o

```

So if you have another C source file `helper.c` with an associated include
file `helper.h` that's used by `test.c`, this will make sure that `test.o` is
built if `helper.h` changes. Implicitly, `test.o` will still build if `test.c`
changes also.
