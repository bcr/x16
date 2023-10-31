#include "cell.h"
#include "ui.h"

int main(void)
{
    cell_ctx ctx = c_init();
    ui_init(ctx);
    return 0;
}
