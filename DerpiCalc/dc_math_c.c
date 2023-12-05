#include <stdio.h>

#include "dc_math.h"

void m_int_to_number(int16_t i, struct number_t *result)
{
    result->number = i;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_symbols_to_number(const uint8_t* s, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t i;
    uint8_t found_decimal = 0;
    uint8_t decimal_count = 0;
    uint8_t is_negative = 0;

    *consumed = 0;
    result->number = 0;

    for (i = 0;i < len;++i)
    {
        if ((i == 0) && ((s[i] == '-') || (s[i] == '+')))
        {
            is_negative = (s[i] == '-');
        }
        else if ((s[i] >= '0') && (s[i] <= '9'))
        {
            result->number *= 10;
            result->number += (s[i] - '0');
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
            result->number /= 10;

    if (is_negative)
        result->number *= -1;

    result->type = NUMBER_TYPE_NORMAL;
}

volatile const char* m_number_to_cstr(const struct number_t* n)
{
    return "FIXME"; // !!! TODO Make work
}

void m_divide(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    result->number = a->number / b->number;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_multiply(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    result->number = a->number * b->number;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_add(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    result->number = a->number + b->number;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_subtract(const struct number_t* a, const struct number_t* b, struct number_t* result)
{
    result->number = a->number - b->number;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_abs(const struct number_t* a, struct number_t* result)
{
    result->number = a->number < 0 ? (-1 * a->number) : a->number;
    result->type = NUMBER_TYPE_NORMAL;
}

int8_t m_compare(const struct number_t* a, const struct number_t* b)
{
    if (a->number < b->number)
        return -1;
    else if (a->number > b->number)
        return 1;
    else
        return 0;
}

void m_sin(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_cos(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_tan(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_atan(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_log(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_sqr(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_exp(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}

void m_int(const struct number_t* a, struct number_t* result)
{
    result->number = 0;
    result->type = NUMBER_TYPE_NORMAL;
}
