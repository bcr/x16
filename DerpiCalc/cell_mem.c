#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "cell.h"
#include "cell_mem.h"

static struct cell_t *cell_columns[MAX_CELL_COLUMN + 1];

void c_mem_init(void)
{
    memset(cell_columns, 0, sizeof(cell_columns));
}

struct cell_t* c_mem_find_cell(uint8_t col, uint8_t row, uint8_t allocate_if_not_found)
{
    struct cell_t** current = &(cell_columns[col]);
    struct cell_t* new_cell;

    while ((*current) && (((*current)->row < row)))
        current = &((*current)->next);

    if ((*current) && ((*current)->row == row))
        return *current;

    if (allocate_if_not_found)
    {
        new_cell = calloc(1, sizeof(struct cell_t));
        new_cell->col = col;
        new_cell->row = row;

        new_cell->next = *current;
        *current = new_cell;

        return new_cell;
    }

    return NULL;
}

void c_mem_iterate_cells(cell_func func)
{
    uint8_t i;
    struct cell_t* current;

    for (i = 0;i <= MAX_CELL_COLUMN;++i)
        for (current = cell_columns[i];current;current = current->next)
            func(current);
}

void c_mem_iterate_cells_by_row(cell_func func)
{
    uint8_t i, j;
    struct cell_t* current;

    for (j = 0;j <= MAX_CELL_ROW;++j)
        for (i = 0;i <= MAX_CELL_COLUMN;++i)
            for (current = cell_columns[i];((current) && (current->row <= j));current = current->next)
                    if (current->row == j)
                        func(current);
}
