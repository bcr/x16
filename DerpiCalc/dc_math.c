#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "dc_math.h"

static volatile const char* const fout_output = ((char*) 0x0100);
static uint8_t temp_hi;
static uint8_t temp_lo;

static void get_fac(struct number_t* result)
{
    temp_hi = ((uint16_t) result) >> 8;
    temp_lo = ((uint16_t) result) & 0x0ff;
    asm("ldx %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE66"); // MOVMF -- Stores FAC IN Y/X (result), 5 bytes long
}

void m_int_to_number(int16_t i, struct number_t *result)
{
    temp_hi = i >> 8;
    temp_lo = i & 0x0FF;
    asm("ldy %v", temp_lo);
    asm("lda %v", temp_hi);
    asm("JSR $FE03"); // GIVAYF -- puts A/Y into FAC

    get_fac(result);
}

const char* m_number_to_cstr(const struct number_t* n)
{
    temp_hi = ((uint16_t) n) >> 8;
    temp_lo = ((uint16_t) n) & 0x0ff;
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE63"); // MOVFM -- puts Y/A into FAC
    asm("JSR $FE06"); // FOUT -- Puts ASCIIZ of FAC in $0100

    return (const char*) fout_output;
}

static void setup_fac_arg(const struct number_t* a, const struct number_t* b)
{
    temp_hi = ((uint16_t) a) >> 8;
    temp_lo = ((uint16_t) a) & 0x0ff;
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE5D"); // ROMUPK -- move memory to ARG

    temp_hi = ((uint16_t) b) >> 8;
    temp_lo = ((uint16_t) b) & 0x0ff;
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE63"); // MOVFM -- move memory to FAC
}

void m_divide(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    setup_fac_arg(a, b);
    asm("JSR $FE27"); // FDIVT -- FAC = ARG / FAC
    get_fac(result);
}
