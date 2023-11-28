#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "cell.h"
#include "cell_mem.h"

static struct cell_t *cell_root;

void c_mem_init(void)
{
    cell_root = NULL;
}

struct cell_t* c_mem_find_cell(uint8_t col, uint8_t row, uint8_t allocate_if_not_found)
{
    struct cell_t** current = &cell_root;
    struct cell_t* last_cell = NULL;

    while (*current)
    {
        if ((*current)->col >= col)
            break;
        current = &((*current)->right);
    }

    if ((*current) && ((*current)->col == col))
    {
        while ((*current) && ((*current)->row > row))
        {
            last_cell = *current;
            current = &((*current)->up);
        }

        while ((*current) && ((*current)->row < row))
        {
            last_cell = *current;
            current = &((*current)->down);
        }

        if ((*current) && ((*current)->row == row))
        {
            return *current;
        }
    }

    if (allocate_if_not_found)
    {
        struct cell_t* new_cell = calloc(1, sizeof(struct cell_t));
        if (*current)
        {
            if ((*current)->col > col)
            {
                new_cell->right = *current;
            }
            else
            {
                if ((*current)->row < row)
                {
                    new_cell->up = *current;
                    new_cell->down = last_cell;
                }
                else
                {
                    new_cell->down = *current;
                    new_cell->up = last_cell;
                }
            }
        }
        *current = new_cell;
        new_cell->col = col;
        new_cell->row = row;
    }
    else
        return NULL;

    return *current;
}
