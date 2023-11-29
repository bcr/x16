# DerpiCalc

![Screenshot](screenshot.png)

This is an attempt to make VisiCalc for the Commander X16. If Dan Bricklin and
Bob Frankston had a C compiler for the 6502, how much better would their life
have been.

## Building

### llvm-mos

To build using llvm-mos and CMake (my current preference) do normal
CMake things. For instance:

```
mkdir build
cd build
cmake ..
make
```

If llvm-mos can't be found, try `-DCMAKE_PREFIX_PATH=/path/to/llvm-mos` when
invoking CMake.

### CC65 (deprecated)

To build with cc65 (may stop being supported, but works now) use the
Makefile provided. It expects cc65 on your path. If it's not on your path, you
need to modify x16cl65.mak in the parent directory (bottom line is that CC
needs to be able to find the cc65 compiler.)

## References

## Stuff to do

- [ ] Error handling prints BASIC complaint and halts
- [ ] Scientific notation for constants
- [ ] Explicitly positive / negative constants
- [ ] Scrutinize NA and ERROR
- [ ] Blank cell handling (@AVERAGE and @COUNT specifically)
- [ ] More soft float work
- [ ] Handle ^ operator

## Observations on compilers

Right now I am using the [CC65 compiler](https://cc65.github.io/).

But there is a [MOS LLVM / CLANG compiler](https://github.com/llvm-mos/llvm-mos-sdk) also that looks pretty sweet.

I have both running now. The .PRG size for LLVM is 2/3 the size of CC65.

Making LLVM work helped me sort out the mapping between keys and symbols and C
characters.

## Busts out to basic (examples)

* @LN(-1) -- maybe check for negative?
* @TAN(@PI/2) -- hidden division by zero
* @SQRT(-1) -- maybe check for negative?
* @EXP(1234) -- overflow

## Known bugs?

* @INT(0-1.234) -- gives -2, VisiCalc gives -1 (truncation, not rounding)
* @ACOS may be broken. As well as @ASIN
* There are dragons past BK253. You have been warned.
* Unknown operators cause much hang (gets fed to e_symbols_to_number)

## VisiCalc quirks

* `@INT(2^3)` is 7, not 8 for some reason
* Defining a column of numbers in reverse requires lots of recalc. Like if you
  put 1 in A4 and make A3 = A4 + 1 and A2 = A3 + 1 and A1 = A2 + 1 then change
  A4 it needs multiple recalc to get A2 and A1 fixed.

## Random notes

There's a problem right now where the mapping of CIRCUMFLEX ACCENT symbol
mapping needs to be sorted out.

When it's time to implement the ^ operator, this is the function.

```c
void m_raise(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    setup_fac_arg(a, b);
#if __CC65__
    asm("JSR $FE39"); // FPWRT -- FAC = ARG ^ FAC
#else
    asm volatile("JSR $FE39":::"a","c","v","x","y"); // FPWRT -- FAC = ARG ^ FAC
#endif /* __CC65__ */
    get_fac(result);
}
```
