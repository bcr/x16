#include <cbm.h>

#include "ui.h"
#include "ui_internal.h"

#define EDIT_BUFFER_LENGTH 79

static uint8_t edit_buffer[EDIT_BUFFER_LENGTH];
static uint8_t edit_buffer_position;

void ui_edit_line_start(void)
{
    s_set_position(0, 2, LAYER_UI);
    s_put_symbol(' ', INVERSE_COLOR);
    edit_buffer_position = 0;
}

uint8_t ui_edit_line_key(uint8_t key)
{
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
        case CH_ENTER:
            return UI_EDIT_LINE_DONE;
    }

    if (edit_buffer_position >= EDIT_BUFFER_LENGTH)
        return UI_EDIT_LINE_CONTINUE;

    s_set_position(edit_buffer_position, 2, LAYER_UI);
    s_put_symbol(key, NORMAL_COLOR);
    s_put_symbol(' ', INVERSE_COLOR);
    edit_buffer[edit_buffer_position++] = key;
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

