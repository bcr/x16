#include <stdint.h>

#include <cbm.h>

#include "screen.h"
#include "cell.h"

#include "ui.h"

#define CLAMP(X, MAX) ((X) >= (MAX) ? ((X) - (MAX)) : (X))
#define CLAMP_ADD(X, Y, MAX) ((((MAX) - (X)) <= (Y)) ? ((Y) - ((MAX) - (X))): ((X) + (Y)))
#define CLAMP_SUB(X, Y, MAX) (((X) < (Y)) ? ((MAX) - ((Y) - (X))) : ((X) - (Y)))

#define NORMAL_COLOR MAKECOLOR(COLOR_GREEN, COLOR_GRAY1)
#define INVERSE_COLOR MAKECOLOR(COLOR_GRAY1, COLOR_GREEN)
#define TRANSPARENT_COLOR MAKECOLOR(COLOR_BLACK, COLOR_BLACK)

#define LAYER_UI 1
#define LAYER_CELLS 0

#define SYMBOL_SPACE 0x20
#define SYMBOL_LATIN_CAPITAL_LETTER_A 0x41
#define SYMBOL_LATIN_SMALL_LETTER_A 0x01
#define SYMBOL_DIGIT_ZERO 0x30

#define COLUMN_WIDTH 9
#define NUMBER_CELL_COLUMNS ((WIDTH_CHARS - 3) / COLUMN_WIDTH)
#define NUMBER_CELL_ROWS (HEIGHT_CHARS - 4)

#define CANVAS_WIDTH_CHARS 128
#define CANVAS_HEIGHT_CHARS 64

static cell_ctx g_cell_ctx;
static uint8_t active_cell_column, active_cell_row;
static uint8_t ul_cell_column, ul_cell_row;
static int8_t scroll_column, scroll_row;
static uint8_t x_offset, y_offset;

static uint8_t ui_draw_asciiz(const char* asciiz, uint8_t color)
{
    uint8_t symbol;
    uint8_t symbols_drawn = 0;

    while (*asciiz)
    {
        symbol = *asciiz;
        if ((symbol >= 'a') && (symbol <= 'z'))
        {
            symbol = symbol - 'a' + SYMBOL_LATIN_SMALL_LETTER_A;
        }
        else if ((symbol >= 'A') && (symbol <= 'Z'))
        {
            symbol = symbol - 'A' + SYMBOL_LATIN_CAPITAL_LETTER_A;
        }
        else if ((symbol >= ' ') && (symbol <= '?'))
        {
        }
        else if (symbol == '[')
        {
            symbol = 27;
        }
        else if (symbol == ']')
        {
            symbol = 29;
        }
        else if (symbol == '@')
        {
            symbol = 0;
        }
        else
        {
            symbol = '?';
        }
        s_put_symbol(symbol, color);
        ++symbols_drawn;
        ++asciiz;
    }

    return symbols_drawn;
}

static void ui_draw_prompt_line(const char* prompt)
{
    static uint8_t last_prompt_line_length = 0;
    uint8_t this_prompt_line_length;
    uint8_t space_chars_to_draw;

    s_set_position(0, 1, LAYER_UI);
    this_prompt_line_length = ui_draw_asciiz(prompt, INVERSE_COLOR);
    space_chars_to_draw = (last_prompt_line_length > this_prompt_line_length) ? (last_prompt_line_length - this_prompt_line_length) : 0;

    while (space_chars_to_draw--)
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    
    last_prompt_line_length = this_prompt_line_length;
}

static void ui_draw_cell_location(uint8_t col, uint8_t row)
{
    if (col >= 26)
        s_put_symbol((col / 26 - 1) + SYMBOL_LATIN_CAPITAL_LETTER_A, INVERSE_COLOR);
    s_put_symbol(col % 26 + SYMBOL_LATIN_CAPITAL_LETTER_A, INVERSE_COLOR);

    row++;
    if (row >= 100)
        s_put_symbol((row / 100) + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
    if (row >= 10)
        s_put_symbol((row / 10 % 10) + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
    s_put_symbol((row % 10) + SYMBOL_DIGIT_ZERO, INVERSE_COLOR);
    row--;

    s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
}

static void ui_draw_row_headers()
{
    uint8_t j;
    uint8_t start_cell_row = ul_cell_row;
    uint8_t current_cell_row = start_cell_row + 1;

    for (j = 4; j<HEIGHT_CHARS;++j,++current_cell_row)
    {
        s_set_position(0, j, LAYER_UI);
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
        s_set_position(3 + i * COLUMN_WIDTH + COLUMN_WIDTH / 2, 3, LAYER_UI);
        s_put_symbol(current_cell_column < 26 ? SYMBOL_SPACE : (current_cell_column / 26 - 1) + SYMBOL_LATIN_CAPITAL_LETTER_A, INVERSE_COLOR);
        s_put_symbol(current_cell_column % 26 + SYMBOL_LATIN_CAPITAL_LETTER_A, INVERSE_COLOR);
        column += COLUMN_WIDTH;
        ++current_cell_column;
    }
}

static void ui_draw_cell_value(uint8_t cell_column, uint8_t cell_row, uint8_t x, uint8_t y, uint8_t color, uint8_t layer)
{
    uint8_t cell_idx;
    const uint8_t* cell_value;
    uint8_t chars_to_draw = (CANVAS_WIDTH_CHARS - x) < COLUMN_WIDTH ? (CANVAS_WIDTH_CHARS - x) : COLUMN_WIDTH;

    cell_value = c_get_cell_value(g_cell_ctx, cell_column, cell_row);

    for (cell_idx = 0;cell_idx < chars_to_draw;++cell_idx)
        s_put_symbol(cell_value[cell_idx], color);

    if (chars_to_draw < COLUMN_WIDTH)
    {
        s_set_position(0, y, layer);
        for (;cell_idx < COLUMN_WIDTH;++cell_idx)
            s_put_symbol(cell_value[cell_idx], color);
    }
}

static void ui_draw_cells(uint8_t x, uint8_t y, uint8_t start_col, uint8_t end_col, uint8_t start_row, uint8_t end_row)
{
    uint8_t current_cell_row, current_cell_column;
    uint8_t current_x;
    uint8_t current_y;

    for (current_cell_row = start_row;current_cell_row <= end_row;++current_cell_row)
    {
        current_x = x;
        current_y = y + current_cell_row - start_row;

        current_y = CLAMP(current_y, CANVAS_HEIGHT_CHARS);

        s_set_position(current_x, current_y, LAYER_CELLS);
        for (current_cell_column = start_col;current_cell_column <= end_col; ++current_cell_column)
        {
            ui_draw_cell_value(current_cell_column, current_cell_row, current_x, current_y, NORMAL_COLOR, LAYER_CELLS);
            current_x += COLUMN_WIDTH;
            if (current_x >= CANVAS_WIDTH_CHARS)
            {
                current_x -= CANVAS_WIDTH_CHARS;
                s_set_position(current_x, current_y, LAYER_CELLS);
            }
        }
    }
}

static void ui_update_scroll(void)
{
    s_scroll(scroll_column, scroll_row, LAYER_CELLS);
}

static void adjust_active_cell(uint8_t new_active_cell_column, uint8_t new_active_cell_row)
{
    uint8_t old_active_cell_column, old_active_cell_row;
    uint8_t i, x, y;

    old_active_cell_column = active_cell_column;
    old_active_cell_row = active_cell_row;

    active_cell_column = new_active_cell_column;
    active_cell_row = new_active_cell_row;

    if ((old_active_cell_column != active_cell_column) || (old_active_cell_row != active_cell_row))
    {
        x = 3 + old_active_cell_column * COLUMN_WIDTH;
        y = 4 + old_active_cell_row;

        // Put transparent spaces in the old cell location
        s_set_position(x, y, LAYER_UI);
        for (i = 0;i < COLUMN_WIDTH;++i)
        {
            s_put_symbol(SYMBOL_SPACE, TRANSPARENT_COLOR);
        }
    }

    x = 3 + active_cell_column * COLUMN_WIDTH;
    y = 4 + active_cell_row;
    s_set_position(x, y, LAYER_UI);
    ui_draw_cell_value(ul_cell_column + active_cell_column, ul_cell_row + active_cell_row,x, y, INVERSE_COLOR, LAYER_UI);

    x = 0;
    y = 0;
    s_set_position(x, y, LAYER_UI);
    ui_draw_cell_location(ul_cell_column + active_cell_column, ul_cell_row + active_cell_row);
}

void ui_init(cell_ctx ctx)
{
    uint8_t i;
    uint8_t j;

    g_cell_ctx = ctx;

    s_init();
    s_clear(TRANSPARENT_COLOR, LAYER_UI);
    s_clear(NORMAL_COLOR, LAYER_CELLS);

    active_cell_column = 0;
    active_cell_row = 0;
    ul_cell_column = 0;
    ul_cell_row = 0;
    scroll_column = -3;
    scroll_row = -4;
    x_offset = 0;
    y_offset = 0;

    s_set_position(0, 0, LAYER_UI);
    for (i = 0;i < WIDTH_CHARS;++i)
    {
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    }

    s_set_position(0, 1, LAYER_UI);
    for (i = 0;i < WIDTH_CHARS;++i)
    {
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    }

    s_set_position(0, 2, LAYER_UI);
    for (i = 0;i < WIDTH_CHARS;++i)
    {
        s_put_symbol(SYMBOL_SPACE, NORMAL_COLOR);
    }

    s_set_position(0, 3, LAYER_UI);
    for (i = 0;i < ((NUMBER_CELL_COLUMNS * COLUMN_WIDTH) + 3);++i)
    {
        s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
    }

    for (j = 3; j<HEIGHT_CHARS;++j)
    {
        s_set_position(0, j, LAYER_UI);
        for (i = 0;i < 3;++i)
        {
            s_put_symbol(SYMBOL_SPACE, INVERSE_COLOR);
        }

        s_set_position(3 + (NUMBER_CELL_COLUMNS * COLUMN_WIDTH), j, LAYER_UI);
        for (i = 3 + (NUMBER_CELL_COLUMNS * COLUMN_WIDTH);i < WIDTH_CHARS;++i)
        {
            s_put_symbol(SYMBOL_SPACE, NORMAL_COLOR);
        }
    }

    ui_draw_row_headers();
    ui_draw_column_headers();
    ui_draw_prompt_line("DerpiCalc (C) 2023 bcr");
    ui_update_scroll();
    ui_draw_cells(x_offset, y_offset, 0, NUMBER_CELL_COLUMNS, 0, NUMBER_CELL_ROWS);
    adjust_active_cell(active_cell_column, active_cell_row);
}

#define COMPUTE_BB() { ul_x = CLAMP_SUB(x_offset, COLUMN_WIDTH, CANVAS_WIDTH_CHARS); ul_y = CLAMP_SUB(y_offset, 1, CANVAS_HEIGHT_CHARS); br_x = CLAMP_ADD(x_offset, (COLUMN_WIDTH * NUMBER_CELL_COLUMNS), CANVAS_WIDTH_CHARS); br_y = CLAMP_ADD(y_offset, NUMBER_CELL_ROWS, CANVAS_HEIGHT_CHARS);}

static void ui_scroll(uint8_t key)
{
    uint8_t ul_x, ul_y, br_x, br_y;
    switch (key)
    {
        case CH_CURS_DOWN:
            ul_cell_row += 1;
            scroll_row += 1;
            y_offset = CLAMP_ADD(y_offset, 1, CANVAS_HEIGHT_CHARS);

            COMPUTE_BB()

            ui_draw_cells(
                (ul_cell_column > 0) ? ul_x : x_offset,
                br_y,
                (ul_cell_column > 0) ? ul_cell_column - 1 : ul_cell_column,
                ul_cell_column + NUMBER_CELL_COLUMNS,
                ul_cell_row + NUMBER_CELL_ROWS,
                ul_cell_row + NUMBER_CELL_ROWS
            );
            ui_draw_row_headers();
            break;
        case CH_CURS_UP:
            if (ul_cell_row > 0)
            {
                ul_cell_row -= 1;
                scroll_row -= 1;
                y_offset = CLAMP_SUB(y_offset, 1, CANVAS_HEIGHT_CHARS);

                if (ul_cell_row > 0)
                {
                    COMPUTE_BB()

                    ui_draw_cells(
                        (ul_cell_column > 0) ? ul_x : x_offset,
                        ul_y,
                        (ul_cell_column > 0) ? ul_cell_column - 1 : ul_cell_column,
                        ul_cell_column + NUMBER_CELL_COLUMNS,
                        ul_cell_row - 1,
                        ul_cell_row - 1
                    );
                }
                ui_draw_row_headers();
            }
            break;
        case CH_CURS_RIGHT:
            ul_cell_column += 1;
            scroll_column += COLUMN_WIDTH;
            x_offset = CLAMP_ADD(x_offset, COLUMN_WIDTH, CANVAS_WIDTH_CHARS);

            COMPUTE_BB()

            ui_draw_cells(
                br_x,
                (ul_cell_row > 0) ? ul_y : y_offset,
                ul_cell_column + NUMBER_CELL_COLUMNS,
                ul_cell_column + NUMBER_CELL_COLUMNS,
                (ul_cell_row > 0) ? ul_cell_row - 1 : ul_cell_row,
                ul_cell_row + NUMBER_CELL_ROWS
            );
            ui_draw_column_headers();
            break;
        case CH_CURS_LEFT:
            if (ul_cell_column > 0)
            {
                ul_cell_column -= 1;
                scroll_column -= COLUMN_WIDTH;
                x_offset = CLAMP_SUB(x_offset, COLUMN_WIDTH, CANVAS_WIDTH_CHARS);

                if (ul_cell_column > 0)
                {
                    COMPUTE_BB()

                    ui_draw_cells(
                        ul_x,
                        (ul_cell_row > 0) ? ul_y : y_offset,
                        ul_cell_column - 1,
                        ul_cell_column - 1,
                        (ul_cell_row > 0) ? ul_cell_row - 1 : ul_cell_row,
                        ul_cell_row + NUMBER_CELL_ROWS
                    );
                }
                ui_draw_column_headers();
            }
            break;
    }

    ui_update_scroll();
}

static void ui_arrows(uint8_t key)
{
    uint8_t do_scroll = 0;
    uint8_t new_active_cell_column = active_cell_column;
    uint8_t new_active_cell_row = active_cell_row;

    switch (key)
    {
        case CH_CURS_DOWN:
            do_scroll = (active_cell_row == (NUMBER_CELL_ROWS - 1));
            if (!do_scroll)
            {
                new_active_cell_row++;
            }
            break;
        case CH_CURS_UP:
            do_scroll = (active_cell_row == 0);
            if (!do_scroll)
            {
                new_active_cell_row--;
            }
            break;
        case CH_CURS_RIGHT:
            do_scroll = (new_active_cell_column == (NUMBER_CELL_COLUMNS - 1));
            if (!do_scroll)
            {
                new_active_cell_column++;
            }
            break;
        case CH_CURS_LEFT:
            do_scroll = (new_active_cell_column == 0);
            if (!do_scroll)
            {
                new_active_cell_column--;
            }
            break;
    }

    if (do_scroll)
        ui_scroll(key);
    adjust_active_cell(new_active_cell_column, new_active_cell_row);
}

void ui_kb(uint8_t key)
{
    switch (key)
    {
        case CH_CURS_DOWN:
        case CH_CURS_UP:
        case CH_CURS_RIGHT:
        case CH_CURS_LEFT:
            ui_arrows(key);
            break;
        default:
            break;
    }
}
