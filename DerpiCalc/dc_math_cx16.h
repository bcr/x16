#ifndef DC_MATH_CX16_H
#define DC_MATH_CX16_H

#include <stdint.h>

#define BASIC_FLOAT_LENGTH 5

struct number_t
{
    uint8_t type;
    uint8_t number[BASIC_FLOAT_LENGTH];
};

#endif /* DC_MATH_CX16_H */
