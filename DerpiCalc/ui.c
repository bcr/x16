#include <stdint.h>

#include <cx16.h>

#include "screen.h"
#include "cell.h"

#include "ui.h"

#define NORMAL_COLOR MAKECOLOR(COLOR_GREEN, COLOR_BLACK)
#define INVERSE_COLOR MAKECOLOR(COLOR_BLACK, COLOR_GREEN)

#define SYMBOL_SPACE 0x20
#define SYMBOL_LATIN_CAPITAL_LETTER_A 0x41
#define SYMBOL_LATIN_SMALL_LETTER_A 0x01
#define SYMBOL_DIGIT_ZERO 0x30

#define COLUMN_WIDTH 9
#define NUMBER_CELL_COLUMNS ((WIDTH_CHARS - 3) / COLUMN_WIDTH)

static void ui_draw_row_headers(uint8_t start_cell_row)
{
    uint8_t j;
    uint8_t current_cell_row = start_cell_row + 1;

    for (j = 4; j<HEIGHT_CHARS;++j,++current_cell_row)
    {
        s_set_position(0, j);
        s_put_symbol(current_cell_row < 100 ? SYMBOL_SPACE : current_cell_row / 100 + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
        s_put_symbol(current_cell_row < 10 ? SYMBOL_SPACE : current_cell_row / 10 % 10 + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
        s_put_symbol(current_cell_row % 10 + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
    }
}

static void ui_draw_column_headers(uint8_t start_cell_column)
{
    uint8_t column = 3 + (COLUMN_WIDTH / 2);
    uint8_t i;
    uint8_t current_cell_column = start_cell_column;

    for (i = 0;i < NUMBER_CELL_COLUMNS;++i)
    {
        s_set_position(3 + i * COLUMN_WIDTH + COLUMN_WIDTH / 2, 3);
        s_put_symbol(current_cell_column < 26 ? SYMBOL_SPACE : (current_cell_column / 26 - 1) + SYMBOL_LATIN_CAPITAL_LETTER_A, INVERSE_COLOR);
        s_put_symbol(current_cell_column % 26 + SYMBOL_LATIN_CAPITAL_LETTER_A, INVERSE_COLOR);
        column += COLUMN_WIDTH;
        ++current_cell_column;
    }
}

static void ui_draw_cells(cell_ctx ctx, uint8_t start_cell_column, uint8_t start_cell_row)
{
    uint8_t j, current_cell_row, current_cell_column, end_cell_column, cell_idx;
    const uint8_t* cell_value;

    end_cell_column = start_cell_column + NUMBER_CELL_COLUMNS - 1;

    for (current_cell_row = start_cell_row, j = 4;j < HEIGHT_CHARS;++j, ++current_cell_row)
    {
        s_set_position(3, j);
        for (current_cell_column = start_cell_column;current_cell_column <= end_cell_column; ++current_cell_column)
        {
            cell_value = c_get_cell_value(ctx, current_cell_column, current_cell_row);
            for (cell_idx = 0;cell_idx < COLUMN_WIDTH;++cell_idx)
                s_put_symbol(cell_value[cell_idx], NORMAL_COLOR);
        }
    }
}

void ui_init(cell_ctx ctx)
{
    uint8_t i;
    uint8_t j;

    s_init();
    s_clear(NORMAL_COLOR);

    s_set_position(0, 0);
    for (i = 0;i < WIDTH_CHARS;++i)
    {
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    }

    s_set_position(0, 1);
    for (i = 0;i < (WIDTH_CHARS - 2);++i)
    {
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    }

    s_set_position(0, 3);
    for (i = 0;i < ((NUMBER_CELL_COLUMNS * COLUMN_WIDTH) + 3);++i)
    {
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    }

    for (j = 4; j<HEIGHT_CHARS;++j)
    {
        s_set_position(0, j);
        for (i = 0;i < 3;++i)
        {
            s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
        }
    }

    ui_draw_row_headers(0);
    ui_draw_column_headers(0);
    ui_draw_cells(ctx, 0, 0);
}
