#include <cbm.h>

#include "input.h"
#include "keyboard.h"
#include "ui.h"

static void in_handle_command()
{
    uint8_t key;

    ui_draw_prompt_line("Command: BCDGIMORV-");
    key = kb_getch();

    switch (key)
    {
        case 'v':
        case 'V':
            ui_draw_prompt_line("DerpiCalc (C) 2023 bcr");
            break;
        default:
            // Unknown command letter
            ui_draw_prompt_line("");
            break;
    }
}

static const uint8_t* in_handle_edit_line(const char* prompt, uint8_t key, uint8_t* rc, uint8_t* len)
{
    const uint8_t* final_string;

    ui_draw_prompt_line(prompt);
    ui_edit_line_start();
    if (key)
        ui_edit_line_key(key);
    while (1)
    {
        key = kb_getch();
        *rc = ui_edit_line_key(key);
        if (*rc)
            break;
    }
    final_string = ui_edit_line_done(len);
    ui_draw_prompt_line("");

    return final_string;
}

static void in_handle_label_entry(uint8_t key)
{
    const uint8_t* final_string;
    uint8_t final_string_length;
    uint8_t rc;

    final_string = in_handle_edit_line("Label", (key == '"') ? 0 : key, &rc, &final_string_length);
}

static void in_handle_value_entry(uint8_t key)
{
    const uint8_t* final_string;
    uint8_t final_string_length;
    uint8_t rc;

    final_string = in_handle_edit_line("Value", key, &rc, &final_string_length);
}

void in_loop(void)
{
    uint8_t key;

    while (1)
    {
        key = kb_getch();
        switch (key)
        {
            case CH_CURS_DOWN:
            case CH_CURS_UP:
            case CH_CURS_RIGHT:
            case CH_CURS_LEFT:
                ui_arrows(key);
                break;
            case '/':
                in_handle_command();
                break;
            default:
                break;
        }

        if (((key >= 'a') && (key <= 'z')) || ((key >= 'A') && (key <= 'Z')) || (key == '"'))
        {
            in_handle_label_entry(key);
        }

        if ((key == '+') || ((key >= '0') && (key <= '9')))
        {
            in_handle_value_entry(key);
        }
    }
}
