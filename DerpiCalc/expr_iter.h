#ifndef EXPR_ITER_H
#define EXPR_ITER_H

#include <stdint.h>

struct range_iter
{
    uint8_t col1;
    uint8_t col2;
    uint8_t row1;
    uint8_t row2;
    uint8_t first;

    int8_t direction;

    // Permissible for caller to inspect
    uint8_t current_col;
    uint8_t current_row;
    uint8_t last;
};

struct comma_iter
{
    const uint8_t* buffer;
    uint8_t buffer_len;
    uint8_t first;

    uint8_t pos;
    uint8_t len;
    uint8_t last;
};

typedef uint8_t (*number_func)(void* state, const struct number_t* number);

uint8_t maybe_parse_range(const uint8_t* buffer, uint8_t len, struct range_iter* iter);
uint8_t range_start(struct range_iter *iter, uint16_t start, uint16_t end);
void range_next(struct range_iter *iter);
void comma_start(struct comma_iter* iter, const uint8_t* buffer, uint8_t len);
void comma_next(struct comma_iter* iter);
uint8_t number_iter(const uint8_t* buffer, uint8_t len, void* state, number_func func);

#endif /* EXPR_ITER_H */
