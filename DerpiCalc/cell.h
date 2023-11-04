#ifndef CELL_H
#define CELL_H

#include <stdint.h>

void c_init(void);
const uint8_t* c_get_cell_value(uint8_t col, uint8_t row);
void c_set_cell_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len);
void c_set_cell_repeating_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len);

#endif /* CELL_H */
