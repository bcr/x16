#include <stdlib.h>

#include "util.h"
#include "cell.h"
#include "cell_mem.h"

#define COLUMNS_PER_BLOCK 8
#define ROWS_PER_BLOCK 64

#define NUMBER_HORIZONTAL_BLOCKS ((MAX_CELL_COLUMN + (COLUMNS_PER_BLOCK - 1)) / COLUMNS_PER_BLOCK)
#define NUMBER_VERTICAL_BLOCKS ((MAX_CELL_ROW + (ROWS_PER_BLOCK - 1)) / ROWS_PER_BLOCK)

static struct cell_t *cell_map [NUMBER_HORIZONTAL_BLOCKS][NUMBER_VERTICAL_BLOCKS];

void c_mem_init(void)
{
    uint8_t x, y;

    for (x = 0;x<NUMBER_HORIZONTAL_BLOCKS;x++)
        for (y = 0;y<NUMBER_VERTICAL_BLOCKS;y++)
            cell_map[x][y] = NULL;
}

struct cell_t* c_mem_find_cell(uint8_t col, uint8_t row, uint8_t allocate_if_not_found)
{
    struct cell_t** current = &(cell_map[col / COLUMNS_PER_BLOCK][row / ROWS_PER_BLOCK]);

    while (*current)
    {
        if (((*current)->col == col) && ((*current)->row == row))
            break;
        current = &((*current)->next);
    }

    if ((*current == NULL) && (allocate_if_not_found))
    {
        struct cell_t* new_cell = calloc(1, sizeof(struct cell_t));
        *current = new_cell;
        new_cell->col = col;
        new_cell->row = row;
    }

    return *current;
}
