#ifndef UI_H
#define UI_H

#include "cell.h"

void ui_init(cell_ctx ctx);
void ui_arrows(uint8_t key);
void ui_draw_prompt_line(const char* prompt);

#endif UI_H
