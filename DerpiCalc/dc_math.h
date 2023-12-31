#ifndef DC_MATH_H
#define DC_MATH_H

#include <stdint.h>

#define NUMBER_TYPE_UNINITIALIZED 0
#define NUMBER_TYPE_NORMAL 1
#define NUMBER_TYPE_NA 2
#define NUMBER_TYPE_ERROR 3

#if DC_MATH_CX16
#include "dc_math_cx16.h"
#elif DC_MATH_C
#include "dc_math_c.h"
#else
#error No math specified. Let's go shopping.
#endif /* DC_MATH_C */

void m_int_to_number(int16_t i, struct number_t *result);
void m_symbols_to_number(const uint8_t* s, uint8_t len, struct number_t* result, uint8_t* consumed);
volatile const char* m_number_to_cstr(const struct number_t* n);
void m_divide(const struct number_t* a, const struct number_t* b, struct number_t* result);
void m_multiply(const struct number_t* a, const struct number_t* b, struct number_t* result);
void m_add(const struct number_t* a, const struct number_t* b, struct number_t* result);
void m_subtract(const struct number_t* a, const struct number_t* b, struct number_t* result);
void m_abs(const struct number_t* a, struct number_t* result);
int8_t m_compare(const struct number_t* a, const struct number_t* b);
void m_sin(const struct number_t* a, struct number_t* result);
void m_cos(const struct number_t* a, struct number_t* result);
void m_tan(const struct number_t* a, struct number_t* result);
void m_atan(const struct number_t* a, struct number_t* result);
void m_log(const struct number_t* a, struct number_t* result);
void m_sqr(const struct number_t* a, struct number_t* result);
void m_exp(const struct number_t* a, struct number_t* result);
void m_int(const struct number_t* a, struct number_t* result);

#endif /* DC_MATH_H */
