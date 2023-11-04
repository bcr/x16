#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"

#define SYMBOL_SPACE 0x20
#define SYMBOL_LATIN_CAPITAL_LETTER_A 0x41
#define SYMBOL_LATIN_SMALL_LETTER_A 0x01
#define SYMBOL_DIGIT_ZERO 0x30

#define MAX_CELL_COLUMN 63
#define MAX_CELL_ROW 254

#define COLUMNS_PER_BLOCK 8
#define ROWS_PER_BLOCK 64

#define NUMBER_HORIZONTAL_BLOCKS ((MAX_CELL_COLUMN + (COLUMNS_PER_BLOCK - 1)) / COLUMNS_PER_BLOCK)
#define NUMBER_VERTICAL_BLOCKS ((MAX_CELL_ROW + (ROWS_PER_BLOCK - 1)) / ROWS_PER_BLOCK)

#define CELL_TYPE_LABEL 0
#define CELL_TYPE_VALUE 1
#define CELL_TYPE_REPEATING 2

struct cell_t
{
    uint8_t type;
    uint8_t col;
    uint8_t row;
    uint8_t contents_len;
    uint8_t* contents;
    uint8_t value_len;
    uint8_t* value;
    struct cell_t* next;
};

static struct cell_t *cell_map [NUMBER_HORIZONTAL_BLOCKS][NUMBER_VERTICAL_BLOCKS];

static uint8_t cell_value[] = "         ";

void c_init(void)
{
    uint8_t x, y;

    for (x = 0;x<NUMBER_HORIZONTAL_BLOCKS;x++)
        for (y = 0;y<NUMBER_VERTICAL_BLOCKS;y++)
            cell_map[x][y] = NULL;
}

static struct cell_t* find_cell(uint8_t col, uint8_t row, uint8_t allocate_if_not_found)
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

const uint8_t* c_get_cell_value(uint8_t col, uint8_t row)
{
    struct cell_t* cell;

    cell = find_cell(col, row, 0);
    return (cell) ? cell->value : cell_value;
}

static void cell_update_value(struct cell_t* cell)
{
    uint8_t i;
    uint8_t bytes_to_copy;
    uint8_t bytes_to_alloc = 9; // !!! TODO 9?!

    if (cell->value)
    {
        free(cell->value);
        cell->value = NULL;
        cell->value_len = 0;
    }

    cell->value = malloc(bytes_to_alloc);
    cell->value_len = bytes_to_alloc;

    if (cell->type == CELL_TYPE_REPEATING)
    {
        bytes_to_copy = cell->value_len;
        for (i = 0;i < bytes_to_copy;++i)
            cell->value[i] = cell->contents[i % cell->contents_len];
    }
    else
    {
        bytes_to_copy = (cell->value_len < cell->contents_len) ? cell->value_len : cell->contents_len;
        memcpy(cell->value, cell->contents, bytes_to_copy);

        if (bytes_to_copy < bytes_to_alloc)
            for (i = bytes_to_copy;i < bytes_to_alloc;i++)
                cell->value[i] = SYMBOL_SPACE;
    }
}

static void cell_clear_contents(struct cell_t* cell)
{
    if (cell->contents != NULL)
    {
        free(cell->contents);
        cell->contents = NULL;
        cell->contents_len = 0;
    }
}

static void cell_set_contents(struct cell_t* cell, const uint8_t* contents, uint8_t len)
{
    cell_clear_contents(cell);
    cell->contents = malloc(len);
    cell->contents_len = len;
    memcpy(cell->contents, contents, len);
}

void c_set_cell_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len)
{
    struct cell_t* cell;

    cell = find_cell(col, row, 1);
    cell->type = CELL_TYPE_LABEL;
    cell_set_contents(cell, label, len);
    cell_update_value(cell);
}

void c_set_cell_repeating_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len)
{
    struct cell_t* cell;

    cell = find_cell(col, row, 1);
    cell->type = CELL_TYPE_REPEATING;
    cell_set_contents(cell, label, len);
    cell_update_value(cell);
}
