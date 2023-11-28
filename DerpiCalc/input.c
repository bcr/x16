#include <stdint.h>

#include <cbm.h>

#include "expr.h"
#include "input.h"
#include "keyboard.h"
#include "ui.h"
#include "util.h"

static uint8_t automatic_recalculation = 1;

static const uint8_t* in_handle_edit_line(const char* prompt, uint8_t* key, uint8_t* rc, uint8_t* len)
{
    const uint8_t* final_string;

    ui_draw_prompt_line(prompt);
    ui_edit_line_start();
    if (*key)
        ui_edit_line_key(*key);
    while (1)
    {
        *key = kb_getch();
        *rc = ui_edit_line_key(*key);
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
    uint8_t key = 0;

    final_string = in_handle_edit_line("Label: Repeating", &key, &rc, &final_string_length);
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

    if ((key == CH_ENTER) || (IS_ARROW_KEY(key)))
    {
        cellref = ui_get_active_cell();
        c_blank_cell(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref));
        ui_refresh_active_cell();
        if (IS_ARROW_KEY(key))
            ui_arrows(key);
    }
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
    uint16_t cellref;
    uint8_t format;
    uint8_t do_format = 0;

    ui_draw_prompt_line("Format: D G I L R $ *");
    key = kb_getch();
    ui_draw_prompt_line("");
    cellref = ui_get_active_cell();

    switch (key)
    {
        case KEY_DOLLAR_SIGN:
            do_format = 1;
            format = CELL_FORMAT_DOLLARS;
            break;
        case KEY_ASTERISK:
            do_format = 1;
            format = CELL_FORMAT_GRAPH;
            break;
        case KEY_LATIN_CAPITAL_LETTER_D:
        case KEY_LATIN_SMALL_LETTER_D:
            do_format = 1;
            format = CELL_FORMAT_DEFAULT;
            break;
        case KEY_LATIN_CAPITAL_LETTER_L:
        case KEY_LATIN_SMALL_LETTER_L:
            do_format = 1;
            format = CELL_FORMAT_LEFT;
            break;
        case KEY_LATIN_CAPITAL_LETTER_R:
        case KEY_LATIN_SMALL_LETTER_R:
            do_format = 1;
            format = CELL_FORMAT_RIGHT;
            break;
        default:
            // !!! TODO
            break;
    }
    if (do_format)
    {
        c_set_cell_format(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), format);
        ui_refresh_active_cell();
    }
}

static void in_handle_recalculate(void)
{
    c_recalculate();
    ui_redraw_cells();
}

static void in_handle_global(void)
{
    uint8_t key;

    ui_draw_prompt_line("Global: C O R F");
    key = kb_getch();
    ui_draw_prompt_line("");

    switch (key)
    {
        case KEY_LATIN_CAPITAL_LETTER_R:
        case KEY_LATIN_SMALL_LETTER_R:
            ui_draw_prompt_line("Recalc: A M");
            key = kb_getch();
            ui_draw_prompt_line("");

            switch (key)
            {
                case KEY_LATIN_CAPITAL_LETTER_A:
                case KEY_LATIN_SMALL_LETTER_A:
                    automatic_recalculation = 1;
                    in_handle_recalculate();
                    break;
                case KEY_LATIN_CAPITAL_LETTER_M:
                case KEY_LATIN_SMALL_LETTER_M:
                    automatic_recalculation = 0;
                    break;
            }

            break;
        default:
            // !!! TODO
            break;
    }
}

static void in_handle_goto_coordinate(void)
{
    const uint8_t* final_string;
    uint8_t consumed;
    uint8_t final_string_length;
    uint8_t rc;
    uint16_t cellref;
    uint8_t key;

    key = 0;

    final_string = in_handle_edit_line("Go to: Coordinate", &key, &rc, &final_string_length);
    if ((rc == UI_EDIT_LINE_DONE) && (final_string_length > 0))
    {
        rc = util_symbols_to_cellref(final_string, final_string_length, &consumed, &cellref);
        if (rc == EVALUATE_OK)
        {
            ui_set_active_cell(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref));
        }
    }
}

static void in_handle_command(void)
{
    uint8_t key;

    ui_draw_prompt_line("Command: BCDGIMORV-");
    key = kb_getch();

    switch (key)
    {
        case KEY_LATIN_SMALL_LETTER_B:
        case KEY_LATIN_CAPITAL_LETTER_B:
            in_handle_blank();
            break;
        case KEY_LATIN_SMALL_LETTER_F:
        case KEY_LATIN_CAPITAL_LETTER_F:
            in_handle_format();
            break;
        case KEY_LATIN_SMALL_LETTER_G:
        case KEY_LATIN_CAPITAL_LETTER_G:
            in_handle_global();
            break;
        case KEY_LATIN_SMALL_LETTER_V:
        case KEY_LATIN_CAPITAL_LETTER_V:
            in_handle_version();
            break;
        case KEY_HYPHEN_MINUS:
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

    key = (key == KEY_QUOTATION_MARK) ? 0 : key;
    final_string = in_handle_edit_line("Label", &key, &rc, &final_string_length);
    if (rc == UI_EDIT_LINE_DONE)
    {
        cellref = ui_get_active_cell();
        c_set_cell_label(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), final_string, final_string_length);
        ui_refresh_active_cell();
        if (IS_ARROW_KEY(key))
            ui_arrows(key);
    }
}

static void in_handle_value_entry(uint8_t key)
{
    const uint8_t* final_string;
    uint8_t final_string_length;
    uint8_t rc;
    uint16_t cellref;

    final_string = in_handle_edit_line("Value", &key, &rc, &final_string_length);
    if (rc == UI_EDIT_LINE_DONE)
    {
        cellref = ui_get_active_cell();
        c_set_cell_value(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), final_string, final_string_length);
        if (automatic_recalculation)
        {
            in_handle_recalculate();
        }
        else
            ui_refresh_active_cell();
        if (IS_ARROW_KEY(key))
            ui_arrows(key);
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
            case KEY_SOLIDUS:
                in_handle_command();
                break;
            case KEY_GREATER_THAN_SIGN:
                in_handle_goto_coordinate();
                break;
            case KEY_EXCLAMATION_MARK:
                in_handle_recalculate();
                break;
            default:
                break;
        }

        if (((key >= KEY_LATIN_SMALL_LETTER_A) && (key <= KEY_LATIN_SMALL_LETTER_Z)) ||
            ((key >= KEY_LATIN_CAPITAL_LETTER_A) && (key <= KEY_LATIN_CAPITAL_LETTER_Z)) ||
             (key == KEY_QUOTATION_MARK))
        {
            in_handle_label_entry(key);
        }

        if ((key == KEY_PLUS_SIGN) ||
            ((key >= KEY_DIGIT_ZERO) && (key <= KEY_DIGIT_NINE)))
        {
            in_handle_value_entry(key);
        }
    }
}
