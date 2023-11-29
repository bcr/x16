#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "cell_fmt.h"
#include "cell_mem.h"
#include "dc_math.h"
#include "expr.h"
#include "util.h"

static uint8_t cell_value[] = "         ";

void c_init(void)
{
    c_mem_init();
}

#ifdef SHOW_DUMMY_CELL_VALUES
const uint8_t* c_get_cell_value(uint8_t col, uint8_t row)
{
    uint8_t symbols_output = 0;

    if (col >= 26)
    {
        cell_value[symbols_output] = (col / 26 - 1) + SYMBOL_LATIN_CAPITAL_LETTER_A;
        symbols_output += 1;
    }
    cell_value[symbols_output] = col % 26 + SYMBOL_LATIN_CAPITAL_LETTER_A;
    symbols_output += 1;

    row++;
    if (row >= 100)
    {
        cell_value[symbols_output] = (row / 100) + SYMBOL_DIGIT_ZERO;
        symbols_output += 1;
    }
    if (row >= 10)
    {
        cell_value[symbols_output] = (row / 10 % 10) + SYMBOL_DIGIT_ZERO;
        symbols_output += 1;
    }
    cell_value[symbols_output] = (row % 10) + SYMBOL_DIGIT_ZERO;
    symbols_output += 1;
    row--;

    for (;symbols_output < (COLUMN_WIDTH - 2);++symbols_output)
        cell_value[symbols_output] = ' ';

    cell_value[symbols_output] = col;
    symbols_output += 1;

    cell_value[symbols_output] = row;
    symbols_output += 1;

    return cell_value;
}
#else
const uint8_t* c_get_cell_value(uint8_t col, uint8_t row)
{
    struct cell_t* cell;

    cell = c_mem_find_cell(col, row, 0);
    return (cell) ? cell->value : cell_value;
}
#endif /* SHOW_DUMMY_CELL_VALUES */

static void cell_update_value(struct cell_t* cell)
{
    uint8_t i;
    uint8_t bytes_to_copy;
    uint8_t bytes_to_alloc = COLUMN_WIDTH;
    uint8_t padding;
    const volatile char* cstr_number;

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
    else if (cell->type == CELL_TYPE_VALUE)
    {
        switch (cell->number.type)
        {
            case NUMBER_TYPE_NORMAL:
                cstr_number = m_number_to_cstr(&cell->number);
                for (i = 0;((*(cstr_number + i)) && (i < cell->value_len));++i)
                    cell->value[i] = *(cstr_number + i);
                break;
            case NUMBER_TYPE_NA:
                i = util_convert_cstr_to_symbol(cell->value, cell->value_len, "NA");
                break;
            case NUMBER_TYPE_ERROR:
                i = util_convert_cstr_to_symbol(cell->value, cell->value_len, "ERROR");
                break;
            default:
                i = 0;
                break;
        }
        for (;i < cell->value_len;++i)
            cell->value[i] = SYMBOL_SPACE;

        switch (cell->format)
        {
            case CELL_FORMAT_DOLLARS:
                if (cell->number.type == NUMBER_TYPE_NORMAL)
                    c_format_value(cell->value, cell->value_len, CELL_FORMAT_DOLLARS);
                c_format_value(cell->value, cell->value_len, CELL_FORMAT_RIGHT);
                break;
            case CELL_FORMAT_RIGHT:
            case CELL_FORMAT_DEFAULT:
            case CELL_FORMAT_GRAPH:
                // !!! TODO implement graph
                c_format_value(cell->value, cell->value_len, CELL_FORMAT_RIGHT);
                break;
            case CELL_FORMAT_LEFT:
                break;
        }
    }
    else
    {
        if ((cell->format == CELL_FORMAT_RIGHT) && (cell->contents_len < cell->value_len))
        {
            padding = (cell->value_len - cell->contents_len);
            for (i = 0;i < padding;++i)
                cell->value[i] = SYMBOL_SPACE;
            memcpy(cell->value + i, cell->contents, cell->contents_len);
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

    cell = c_mem_find_cell(col, row, 1);
    cell->type = CELL_TYPE_LABEL;
    cell->number.type = NUMBER_TYPE_UNINITIALIZED;
    cell_set_contents(cell, label, len);
    cell_update_value(cell);
}

static void cell_update_number(struct cell_t* cell)
{
    uint8_t rc = EVALUATE_GENERAL_ERROR;

    cell->number.type = NUMBER_TYPE_UNINITIALIZED;
    rc = e_evaluate(cell->contents, cell->contents_len, &cell->number);
    if (rc != EVALUATE_OK)
        cell->number.type = NUMBER_TYPE_ERROR;
}

void c_set_cell_value(uint8_t col, uint8_t row, const uint8_t* value, uint8_t len)
{
    struct cell_t* cell;

    cell = c_mem_find_cell(col, row, 1);
    cell->type = CELL_TYPE_VALUE;
    cell_set_contents(cell, value, len);
    cell_update_number(cell);
    cell_update_value(cell);
}

void c_set_cell_repeating_label(uint8_t col, uint8_t row, const uint8_t* label, uint8_t len)
{
    struct cell_t* cell;

    cell = c_mem_find_cell(col, row, 1);
    cell->type = CELL_TYPE_REPEATING;
    cell->number.type = NUMBER_TYPE_UNINITIALIZED;
    cell_set_contents(cell, label, len);
    cell_update_value(cell);
}

void c_blank_cell(uint8_t col, uint8_t row)
{
    struct cell_t* cell;
    cell = c_mem_find_cell(col, row, 0);
    if (cell != NULL)
    {
        cell->type = CELL_TYPE_BLANK;
        cell->number.type = NUMBER_TYPE_UNINITIALIZED;
        cell_clear_contents(cell);
        cell_update_value(cell);
    }
}

uint8_t c_get_cell_number(uint8_t col, uint8_t row, struct number_t* result)
{
    struct cell_t* cell;
    cell = c_mem_find_cell(col, row, 0);
    if ((cell != NULL) && (cell->number.type != NUMBER_TYPE_UNINITIALIZED))
    {
        memcpy(result, &cell->number, sizeof(struct number_t));
        return EVALUATE_OK;
    }
    m_int_to_number(0, result);
    return EVALUATE_OK;
}

uint8_t c_get_cell_type(uint8_t col, uint8_t row)
{
    struct cell_t* cell;
    cell = c_mem_find_cell(col, row, 0);
    return (cell != NULL) ? cell->type : CELL_TYPE_BLANK;
}

const uint8_t* c_get_cell_contents(uint8_t col, uint8_t row, uint8_t* contents_len)
{
    struct cell_t* cell;
    cell = c_mem_find_cell(col, row, 0);
    if (cell != NULL)
    {
        *contents_len = cell->contents_len;
        return cell->contents;
    }
    *contents_len = 0;
    return NULL;
}

void c_set_cell_format(uint8_t col, uint8_t row, uint8_t format)
{
    struct cell_t* cell;

    cell = c_mem_find_cell(col, row, 1);
    cell->format = format;
    cell_update_value(cell);
}

uint8_t c_get_cell_format(uint8_t col, uint8_t row)
{
    struct cell_t* cell = c_mem_find_cell(col, row, 0);
    return (cell != NULL) ? cell->format : CELL_FORMAT_DEFAULT;
}

static void recalculate_cell(struct cell_t* cell)
{
    if (cell->type == CELL_TYPE_VALUE)
    {
        cell_update_number(cell);
        cell_update_value(cell);
    }
}

void c_recalculate(uint8_t recalculate_order)
{
    if (recalculate_order == RECALCULATE_ORDER_COLUMNS)
        c_mem_iterate_cells(recalculate_cell);
    else
        c_mem_iterate_cells_by_row(recalculate_cell);
}
