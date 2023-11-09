#include <cbm.h>

#include "input.h"
#include "keyboard.h"
#include "ui.h"

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

static void in_handle_repeating_label_entry(void)
{
    const uint8_t* final_string;
    uint8_t final_string_length;
    uint8_t rc;
    uint16_t cellref;

    final_string = in_handle_edit_line("Label: Repeating", 0, &rc, &final_string_length);
    if ((rc == UI_EDIT_LINE_DONE) && (final_string_length > 0))
    {
        cellref = ui_get_active_cell();
        c_set_cell_repeating_label(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), final_string, final_string_length);
        ui_refresh_active_cell();
    }
}

static void in_handle_blank(void)
{
    uint8_t key;
    uint16_t cellref;

    ui_draw_prompt_line("Blank");
    key = kb_getch();
    ui_draw_prompt_line("");

    if (key == CH_ENTER)
    {
        cellref = ui_get_active_cell();
        c_blank_cell(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref));
        ui_refresh_active_cell();
    }
    // !!! TODO
    // DOS VisiCalc:
    // CH_ENTER: Blank it
    // Arrow keys: Blank it and move active cell
    // Anything else: Beep and don't blank it
}

static void in_handle_version(void)
{
    uint8_t key;
    ui_draw_prompt_line("DerpiCalc (C) 2023 bcr");    
    key = kb_getch();
    ui_draw_prompt_line("");

    // !!! TODO
    // DOS VisiCalc:
    // Keep message until a key pressed, do normal key activity
}

static void in_handle_format(void)
{
    uint8_t key;
    ui_draw_prompt_line("Format: D G I L R $ *");
    key = kb_getch();
    ui_draw_prompt_line("");

    // !!! TODO
}

static void in_handle_command(void)
{
    uint8_t key;

    ui_draw_prompt_line("Command: BCDGIMORV-");
    key = kb_getch();

    switch (key)
    {
        case 'b':
        case 'B':
            in_handle_blank();
            break;
        case 'f':
        case 'F':
            in_handle_format();
            break;
        case 'v':
        case 'V':
            in_handle_version();
            break;
        case '-':
            in_handle_repeating_label_entry();
            break;
        default:
            // Unknown command letter
            ui_draw_prompt_line("");
            break;
    }
}

static void in_handle_label_entry(uint8_t key)
{
    const uint8_t* final_string;
    uint8_t final_string_length;
    uint8_t rc;
    uint16_t cellref;

    final_string = in_handle_edit_line("Label", (key == '"') ? 0 : key, &rc, &final_string_length);
    if (rc == UI_EDIT_LINE_DONE)
    {
        cellref = ui_get_active_cell();
        c_set_cell_label(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), final_string, final_string_length);
        ui_refresh_active_cell();
    }
}

static void in_handle_value_entry(uint8_t key)
{
    const uint8_t* final_string;
    uint8_t final_string_length;
    uint8_t rc;
    uint16_t cellref;

    final_string = in_handle_edit_line("Value", key, &rc, &final_string_length);
    if (rc == UI_EDIT_LINE_DONE)
    {
        cellref = ui_get_active_cell();
        c_set_cell_value(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), final_string, final_string_length);
        ui_refresh_active_cell();
    }
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
