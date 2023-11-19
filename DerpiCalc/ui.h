#ifndef UI_H
#define UI_H

#include "cell.h"

#define UI_EDIT_LINE_CONTINUE 0
#define UI_EDIT_LINE_DONE 1
#define UI_EDIT_LINE_CANCELED 2

void ui_init(void);
void ui_arrows(uint8_t key);
void ui_draw_prompt_line(const char* prompt);
uint16_t ui_get_active_cell(void);
void ui_refresh_active_cell(void);

void ui_edit_line_start(void);
uint8_t ui_edit_line_key(uint8_t key);
const uint8_t* ui_edit_line_done(uint8_t* len);
void ui_set_active_cell(uint8_t new_active_cell_column, uint8_t new_active_cell_row);

#endif /* UI_H */
