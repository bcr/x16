#include <cbm.h>

#include "ui.h"
#include "ui_internal.h"
#include "util.h"

#define EDIT_BUFFER_LENGTH 79

static uint8_t edit_buffer[EDIT_BUFFER_LENGTH];
static uint8_t edit_buffer_position;

void ui_edit_line_start(void)
{
    s_set_position(0, 2, LAYER_UI);
    s_put_symbol(' ', INVERSE_COLOR);
    edit_buffer_position = 0;
}

static uint8_t symbol_from_key(uint8_t key)
{
    uint8_t symbol = key;

    if ((symbol >= KEY_LATIN_SMALL_LETTER_A) && (symbol <= KEY_LATIN_SMALL_LETTER_Z))
    {
        symbol = symbol - KEY_LATIN_SMALL_LETTER_A + SYMBOL_LATIN_SMALL_LETTER_A;
    }
    else if ((symbol >= KEY_LATIN_CAPITAL_LETTER_A) && (symbol <= KEY_LATIN_CAPITAL_LETTER_Z))
    {
        symbol = symbol - KEY_LATIN_CAPITAL_LETTER_A + SYMBOL_LATIN_CAPITAL_LETTER_A;
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

    return symbol;
}

uint8_t ui_edit_line_key(uint8_t key)
{
    uint8_t symbol;

    switch (key)
    {
        case CH_ESC:
        case CH_DEL:
            if (edit_buffer_position > 0)
            {
                edit_buffer_position--;
                s_set_position(edit_buffer_position, 2, LAYER_UI);
                s_put_symbol(' ', INVERSE_COLOR);
                s_put_symbol(' ', NORMAL_COLOR);
                return UI_EDIT_LINE_CONTINUE;
            }
            return UI_EDIT_LINE_CANCELED;
        case CH_CURS_UP:
        case CH_CURS_DOWN:
        case CH_CURS_LEFT:
        case CH_CURS_RIGHT:
        case CH_ENTER:
            return UI_EDIT_LINE_DONE;
    }

    if (edit_buffer_position >= EDIT_BUFFER_LENGTH)
        return UI_EDIT_LINE_CONTINUE;

    symbol = symbol_from_key(key);
    s_set_position(edit_buffer_position, 2, LAYER_UI);
    s_put_symbol(symbol, NORMAL_COLOR);
    s_put_symbol(' ', INVERSE_COLOR);
    edit_buffer[edit_buffer_position++] = symbol;
    return UI_EDIT_LINE_CONTINUE;
}

const uint8_t* ui_edit_line_done(uint8_t* len)
{
    uint8_t x;

    s_set_position(0, 2, LAYER_UI);
    for (x = 0;x < edit_buffer_position + 1;++x)
        s_put_symbol(' ', NORMAL_COLOR);

    *len = edit_buffer_position;
    return edit_buffer;
}

