#include <stdint.h>

#include <cbm.h>

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

static cell_ctx g_cell_ctx;
static uint8_t active_cell_column, active_cell_row;
static uint8_t ul_cell_column, ul_cell_row;

static void ui_draw_row_headers()
{
    uint8_t j;
    uint8_t start_cell_row = ul_cell_row;
    uint8_t current_cell_row = start_cell_row + 1;

    for (j = 4; j<HEIGHT_CHARS;++j,++current_cell_row)
    {
        s_set_position(0, j);
        s_put_symbol(current_cell_row < 100 ? SYMBOL_SPACE : current_cell_row / 100 + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
        s_put_symbol(current_cell_row < 10 ? SYMBOL_SPACE : current_cell_row / 10 % 10 + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
        s_put_symbol(current_cell_row % 10 + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
    }
}

static void ui_draw_column_headers()
{
    uint8_t column = 3 + (COLUMN_WIDTH / 2);
    uint8_t i;
    uint8_t start_cell_column = ul_cell_column;
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

static void ui_draw_cell_value(uint8_t cell_column, uint8_t cell_row)
{
    uint8_t cell_idx, active;
    const uint8_t* cell_value;

    cell_value = c_get_cell_value(g_cell_ctx, cell_column, cell_row);
    active = (active_cell_column == cell_column) && (active_cell_row == cell_row);
    for (cell_idx = 0;cell_idx < COLUMN_WIDTH;++cell_idx)
        s_put_symbol(cell_value[cell_idx], active ? INVERSE_COLOR : NORMAL_COLOR);    
}

static void ui_draw_cells(uint8_t start_cell_column, uint8_t start_cell_row)
{
    uint8_t j, current_cell_row, current_cell_column, end_cell_column;

    end_cell_column = start_cell_column + NUMBER_CELL_COLUMNS - 1;

    for (current_cell_row = start_cell_row, j = 4;j < HEIGHT_CHARS;++j, ++current_cell_row)
    {
        s_set_position(3, j);
        for (current_cell_column = start_cell_column;current_cell_column <= end_cell_column; ++current_cell_column)
        {
            ui_draw_cell_value(current_cell_column, current_cell_row);
        }
    }
}

static void draw_single_cell(uint8_t cell_column, uint8_t cell_row)
{
    s_set_position(3 + ((cell_column - ul_cell_column) * COLUMN_WIDTH), cell_row - ul_cell_row + 4);
    ui_draw_cell_value(cell_column, cell_row);
}

void ui_init(cell_ctx ctx)
{
    uint8_t i;
    uint8_t j;

    g_cell_ctx = ctx;

    s_init();
    s_clear(NORMAL_COLOR);

    active_cell_column = 0;
    active_cell_row = 0;
    ul_cell_column = 0;
    ul_cell_row = 0;

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

    ui_draw_row_headers();
    ui_draw_column_headers();
    ui_draw_cells(0, 0);
}

static void adjust_active_cell(uint8_t new_active_cell_column, uint8_t new_active_cell_row)
{
    uint8_t old_active_cell_column, old_active_cell_row;

    old_active_cell_column = active_cell_column;
    old_active_cell_row = active_cell_row;

    active_cell_column = new_active_cell_column;
    active_cell_row = new_active_cell_row;

    draw_single_cell(old_active_cell_column, old_active_cell_row);
    draw_single_cell(active_cell_column, active_cell_row);
}

void ui_kb(uint8_t key)
{
    uint8_t new_active_cell_column = active_cell_column;
    uint8_t new_active_cell_row = active_cell_row;

    switch (key)
    {
        case CH_CURS_DOWN:
            new_active_cell_row += 1;
            break;
        case CH_CURS_UP:
            if (new_active_cell_row > 0)
                new_active_cell_row -= 1;
            break;
        case CH_CURS_RIGHT:
            new_active_cell_column += 1;
            break;
        case CH_CURS_LEFT:
            if (new_active_cell_column > 0)
                new_active_cell_column -= 1;
            break;
    }

    if ((new_active_cell_column != active_cell_column) || (new_active_cell_row != active_cell_row))
    {
        adjust_active_cell(new_active_cell_column, new_active_cell_row);
    }
}
