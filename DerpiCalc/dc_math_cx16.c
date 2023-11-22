#include "dc_math.h"

static volatile const char* const fout_output = ((char*) 0x0100);
static volatile uint8_t temp_hi;
static volatile uint8_t temp_lo;

static void get_fac(struct number_t* result)
{
    temp_hi = ((uint16_t) result->number) >> 8;
    temp_lo = ((uint16_t) result->number) & 0x0ff;

#if __CC65__
    asm("ldx %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE66"); // MOVMF -- Stores FAC IN Y/X (result), 5 bytes long
#else
    asm volatile("JSR $FE66" :: "x"(temp_lo), "y"(temp_hi):"a","c","v","x","y"); // MOVMF -- Stores FAC IN Y/X (result), 5 bytes long
#endif /* __CC65 */
    result->type = NUMBER_TYPE_NORMAL;
}

static void zero_fac(void)
{
#if __CC65__
    asm("JSR $FE72"); // ZEROFC -- FAC = 0
#else
    asm volatile("JSR $FE72":::"a","c","v","x","y"); // ZEROFC -- FAC = 0
#endif /* __CC65 */
}

static void finlog(int8_t digit)
{
    temp_lo = digit;
#if __CC65__
    asm("lda %v", temp_lo);
    asm("JSR $FE90"); // FINLOG -- add accumulator to FAc
#else
    asm volatile("JSR $FE90" :: "a"(temp_lo):"a","c","v","x","y"); // FINLOG -- add accumulator to FAc
#endif /* __CC65 */
}

static void mul10(void)
{
#if __CC65__
    asm("JSR $FE7B"); // MUL10 -- FAC *= 10
#else
    asm volatile("JSR $FE7B":::"a","c","v","x","y"); // MUL10 -- FAC *= 10
#endif /* __CC65 */
}

static void div10(void)
{
#if __CC65__
    asm("JSR $FE7E"); // DIV10 -- FAC /= 10
#else
    asm volatile("JSR $FE7E":::"a","c","v","x","y"); // DIV10 -- FAC /= 10
#endif /* __CC65 */
}

static void negop(void)
{
#if __CC65__
    asm("JSR $FE33"); // NEGOP -- FAC = -FAC
#else
    asm volatile("JSR $FE33":::"a","c","v","x","y"); // NEGOP -- FAC = -FAC
#endif /* __CC65 */
}

void m_int_to_number(int16_t i, struct number_t *result)
{
    temp_hi = i >> 8;
    temp_lo = i & 0x0FF;
#if __CC65__
    asm("ldy %v", temp_lo);
    asm("lda %v", temp_hi);
    asm("JSR $FE03"); // GIVAYF -- puts A/Y into FAC
#else
    asm volatile("JSR $FE03" :: "y"(temp_lo), "a"(temp_hi):"a","c","v","x","y"); // GIVAYF -- puts A/Y into FAC
#endif /* __CC65 */

    get_fac(result);
}

void m_symbols_to_number(const uint8_t* s, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t i;
    uint8_t found_decimal = 0;
    uint8_t decimal_count = 0;
    uint8_t is_negative = 0;

    *consumed = 0;
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
        else
            break;  // Unknown symbol, stop processing
        ++*consumed;
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
#if __CC65__
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE63"); // MOVFM -- puts Y/A into FAC
    asm("JSR $FE06"); // FOUT -- Puts ASCIIZ of FAC in $0100
#else
    asm volatile("JSR $FE63" :: "a"(temp_lo), "y"(temp_hi):"a","c","v","x","y"); // MOVFM -- puts Y/A into FAC
    asm volatile("JSR $FE06":::"a","c","v","x","y"); // FOUT -- Puts ASCIIZ of FAC in $0100
#endif /* __CC65__ */

    return fout_output;
}

static void setup_fac(const struct number_t* a)
{
    temp_hi = ((uint16_t) a->number) >> 8;
    temp_lo = ((uint16_t) a->number) & 0x0ff;
#if __CC65__
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE63"); // MOVFM -- move memory to FAC
#else
    asm volatile("JSR $FE63" :: "a"(temp_lo), "y"(temp_hi):"a","c","v","x","y"); // MOVFM -- move memory to FAC
#endif /* __CC65__ */
}

static void setup_fac_arg(const struct number_t* a, const struct number_t* b)
{
    temp_hi = ((uint16_t) a->number) >> 8;
    temp_lo = ((uint16_t) a->number) & 0x0ff;
#if __CC65__
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE5D"); // ROMUPK -- move memory to ARG
#else
    asm volatile("JSR $FE5D" :: "a"(temp_lo), "y"(temp_hi):"a","c","v","x","y"); // ROMUPK -- move memory to ARG
#endif /* __CC65__ */

    setup_fac(b);
}

void m_divide(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    setup_fac_arg(a, b);
#if __CC65__
    asm("JSR $FE27"); // FDIVT -- FAC = ARG / FAC
#else
    asm volatile("JSR $FE27":::"a","c","v","x","y"); // FDIVT -- FAC = ARG / FAC
#endif /* __CC65__ */
    get_fac(result);
}

void m_multiply(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    setup_fac_arg(a, b);
#if __CC65__
    asm("JSR $FE21"); // FMULTT -- FAC = ARG * FAC
#else
    asm volatile("JSR $FE21":::"a","c","v","x","y"); // FMULTT -- FAC = ARG * FAC
#endif /* __CC65__ */
    get_fac(result);
}

void m_add(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    setup_fac_arg(a, b);
#if __CC65__
    asm("JSR $FE1B"); // FADDT -- FAC = ARG + FAC
#else
    asm volatile("JSR $FE1B":::"a","c","v","x","y"); // FADDT -- FAC = ARG + FAC
#endif /* __CC65__ */
    get_fac(result);
}

void m_subtract(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    setup_fac_arg(a, b);
#if __CC65__
    asm("JSR $FE15"); // FSUBT -- FAC = ARG - FAC
#else
    asm volatile("JSR $FE15":::"a","c","v","x","y"); // FSUBT -- FAC = ARG - FAC
#endif /* __CC65__ */
    get_fac(result);
}

void m_abs(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE4E"); // ABS -- FAC = ABS(FAC)
#else
    asm volatile("JSR $FE4E":::"a","c","v","x","y"); // ABS -- FAC = ABS(FAC)
#endif /* __CC65__ */
    get_fac(result);
}

int8_t m_compare(const struct number_t* a, const struct number_t* b)
{
    setup_fac(a);
    temp_hi = ((uint16_t) b->number) >> 8;
    temp_lo = ((uint16_t) b->number) & 0x0ff;
#if __CC65__
    asm("lda %v", temp_lo);
    asm("ldy %v", temp_hi);
    asm("JSR $FE54"); // FCOMP -- Compare
    asm("sta %v", temp_lo);
#else
    asm volatile("JSR $FE54" : "=a"(temp_lo) : "a"(temp_lo), "y"(temp_hi):"a","c","v","x","y"); // FCOMP -- Compare
#endif /* __CC65__ */

    return temp_lo;
}

void m_sin(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE42"); // SIN -- FAC = SIN(FAC)
#else
    asm volatile("JSR $FE42":::"a","c","v","x","y"); // SIN -- FAC = SIN(FAC)
#endif /* __CC65__ */
    get_fac(result);
}

void m_cos(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE3F"); // COS -- FAC = COS(FAC)
#else
    asm volatile("JSR $FE3F":::"a","c","v","x","y"); // COS -- FAC = COS(FAC)
#endif /* __CC65__ */
    get_fac(result);
}

void m_tan(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE45"); // TAN -- FAC = TAN(FAC)
#else
    asm volatile("JSR $FE45":::"a","c","v","x","y"); // TAN -- FAC = TAN(FAC)
#endif /* __CC65__ */
    get_fac(result);
}

void m_atan(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE48"); // ATN -- FAC = ATAN(FAC)
#else
    asm volatile("JSR $FE48":::"a","c","v","x","y"); // ATN -- FAC = ATAN(FAC)
#endif /* __CC65__ */
    get_fac(result);
}

void m_log(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE2A"); // LOG -- FAC = LOG(FAC) (natural log)
#else
    asm volatile("JSR $FE2A":::"a","c","v","x","y"); // LOG -- FAC = LOG(FAC) (natural log)
#endif /* __CC65__ */
    get_fac(result);
}

void m_sqr(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE30"); // SQR -- FAC = SQR(FAC) (square root)
#else
    asm volatile("JSR $FE30":::"a","c","v","x","y"); // SQR -- FAC = SQR(FAC) (square root)
#endif /* __CC65__ */
    get_fac(result);
}

void m_exp(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE3C"); // EXP -- FAC = EXP(FAC) (exponent)
#else
    asm volatile("JSR $FE3C":::"a","c","v","x","y"); // EXP -- FAC = EXP(FAC) (exponent)
#endif /* __CC65__ */
    get_fac(result);
}

void m_int(const struct number_t* a, struct number_t* result)
{
    setup_fac(a);
#if __CC65__
    asm("JSR $FE2D"); // INT -- FAC = INT(FAC) (int truncate)
#else
    asm volatile("JSR $FE2D":::"a","c","v","x","y"); // INT -- FAC = INT(FAC) (int truncate)
#endif /* __CC65__ */
    get_fac(result);
}
