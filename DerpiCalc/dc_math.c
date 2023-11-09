#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "dc_math.h"

static volatile const char* const fout_output = ((char*) 0x0100);
static uint8_t temp_hi;
static uint8_t temp_lo;

static void get_fac(struct number_t* result)
{
    temp_hi = ((uint16_t) result->number) >> 8;
    temp_lo = ((uint16_t) result->number) & 0x0ff;
    asm("ldx %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE66"); // MOVMF -- Stores FAC IN Y/X (result), 5 bytes long
}

static void zero_fac(void)
{
    asm("JSR $FE72"); // ZEROFC -- FAC = 0
}

static void finlog(int8_t digit)
{
    temp_lo = digit;
    asm("lda %v", temp_lo);
    asm("JSR $FE90"); // FINLOG -- add accumulator to FAc
}

static void mul10(void)
{
    asm("JSR $FE7B"); // MUL10 -- FAC *= 10
}

static void div10(void)
{
    asm("JSR $FE7E"); // DIV10 -- FAC /= 10
}

static void negop(void)
{
    asm("JSR $FE33"); // NEGOP -- FAC = -FAC
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

void m_symbols_to_number(const uint8_t* s, uint8_t len, struct number_t* result)
{
    uint8_t i;
    uint8_t found_decimal = 0;
    uint8_t decimal_count = 0;
    uint8_t is_negative = 0;

    zero_fac();

    for (i = 0;i < len;++i)
    {
        if ((i == 0) && ((s[i] == '-') || (s[i] == '+')))
        {
            is_negative = (s[i] == '-');
        }
        else if ((s[i] >= '0') && (s[i] <= '9'))
        {
            mul10();
            finlog(s[i] - '0');
            decimal_count++;
        }
        else if (s[i] == '.')
        {
            // !!! TODO Maybe complain if found_decimal is already 1
            found_decimal = 1;
            decimal_count = 0;
        }
    }

    if (found_decimal)
        for (i = 0;i < decimal_count;++i)
            div10();

    if (is_negative)
        negop();

    get_fac(result);
}

volatile const char* m_number_to_cstr(const struct number_t* n)
{
    temp_hi = ((uint16_t) n->number) >> 8;
    temp_lo = ((uint16_t) n->number) & 0x0ff;
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE63"); // MOVFM -- puts Y/A into FAC
    asm("JSR $FE06"); // FOUT -- Puts ASCIIZ of FAC in $0100

    return fout_output;
}

static void setup_fac_arg(const struct number_t* a, const struct number_t* b)
{
    temp_hi = ((uint16_t) a->number) >> 8;
    temp_lo = ((uint16_t) a->number) & 0x0ff;
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE5D"); // ROMUPK -- move memory to ARG

    temp_hi = ((uint16_t) b->number) >> 8;
    temp_lo = ((uint16_t) b->number) & 0x0ff;
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
