# Commander X16 Explorations

## C Code

### Makefile Shenanigans

I have included a `Makefile` that may be instructive:

```make
CC = cl65
TARGET_ARCH = -t cx16
```

The important things are setting the `CC` and `TARGET_ARCH` Other than this,
everything should behave the normal way `make` does things. The rest of this
section is a poor review of how `make` works.

If you have a single source file `test.c` and you add:

```make
all: test
```

It will build that source file into `test` -- the `all` target is just for
convenience if you add more targets, as `make` will by default build the first
target.

You can do something more complicated like:

```make
all: test

test.o: helper.h

test.prg: test.o helper.o

```

So if you have another C source file `helper.c` with an associated include
file `helper.h` that's used by `test.c`, this will make sure that `test.o` is
built if `helper.h` changes. Implicitly, `test.o` will still build if `test.c`
changes also.

### Loading the Final Output File

The final output file will be in the `PRG` format and is compatible with the
`LOAD` command. You can load and run it when firing up `x16emu`:

```
x16emu -prg /path/to/test -run
```
