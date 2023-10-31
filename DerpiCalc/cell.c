#include <stddef.h>

#include "cell.h"

cell_ctx c_init(void)
{
    return 1;
}

static uint8_t cell_value[] = "         ";

const uint8_t* c_get_cell_value(cell_ctx ctx, uint8_t col, uint8_t row)
{
    cell_value[0] = col;
    cell_value[1] = row;
    return cell_value;
}
