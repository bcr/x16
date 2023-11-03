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
    }
}
