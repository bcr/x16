#ifndef CELL_H
#define CELL_H

#include <stdint.h>

#include "dc_math.h"

#define CELL_TYPE_BLANK 0
#define CELL_TYPE_LABEL 1
#define CELL_TYPE_VALUE 2
#define CELL_TYPE_REPEATING 3

#define CELL_FORMAT_DEFAULT 0
#define CELL_FORMAT_DOLLARS 1
#define CELL_FORMAT_RIGHT 2
#define CELL_FORMAT_LEFT 3
#define CELL_FORMAT_GRAPH 4

void c_init(void);
const uint8_t* c_get_cell_value(uint8_t col, uint8_t row);
void c_set_cell_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len);
void c_set_cell_value(uint8_t col, uint8_t row, const uint8_t* value, uint8_t len);
void c_set_cell_repeating_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len);
void c_blank_cell(uint8_t col, uint8_t row);
uint8_t c_get_cell_number(uint8_t col, uint8_t row, struct number_t* result);
uint8_t c_get_cell_type(uint8_t col, uint8_t row);
const uint8_t* c_get_cell_contents(uint8_t col, uint8_t row, uint8_t* contents_len);
void c_set_cell_format(uint8_t col, uint8_t row, uint8_t format);
uint8_t c_get_cell_format(uint8_t col, uint8_t row);
void c_recalculate(uint8_t recalculate_order);

#endif /* CELL_H */
