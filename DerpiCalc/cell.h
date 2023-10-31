#ifndef CELL_H
#define CELL_H

#include <stdint.h>

typedef int cell_ctx;

cell_ctx c_init(void);
const uint8_t* c_get_cell_value(cell_ctx ctx, uint8_t col, uint8_t row);

#endif /* CELL_H */
