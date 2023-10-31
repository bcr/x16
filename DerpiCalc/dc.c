#include "cell.h"
#include "keyboard.h"
#include "ui.h"

int main(void)
{
    uint8_t key;

    cell_ctx ctx = c_init();
    kb_init();
    ui_init(ctx);
    while (1)
    {
        key = kb_getch();
        ui_kb(key);
    }
    return 0;
}
