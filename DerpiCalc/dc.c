#include "cell.h"
#include "input.h"
#include "keyboard.h"
#include "ui.h"

int main(void)
{
    cell_ctx ctx = c_init();
    kb_init();
    ui_init(ctx);
    in_loop();
    return 0;
}
