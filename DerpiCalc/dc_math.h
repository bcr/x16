#ifndef DC_MATH_H
#define DC_MATH_H

#include <stdint.h>

#define BASIC_FLOAT_LENGTH 5
struct number_t
{
    uint8_t number[BASIC_FLOAT_LENGTH];
};

void m_int_to_number(int16_t i, struct number_t *result);
volatile const char* m_number_to_cstr(const struct number_t* n);
void m_divide(const struct number_t* a, const struct number_t* b, struct number_t* result);

#endif /* DC_MATH_H */
