#include "cell.h"
#include "input.h"
#include "keyboard.h"
#include "ui.h"

int main(void)
{
    c_init();
    kb_init();
    ui_init();
    in_loop();
    return 0;
}
