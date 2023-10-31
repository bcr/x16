#ifndef SCREEN_H
#define SCREEN_H

#define WIDTH_CHARS 80
#define HEIGHT_CHARS 60

#define MAKECOLOR(FG, BG) ((FG) | (BG) << 4)

void s_init(void);
void s_clear(uint8_t color, uint8_t layer);
void s_set_position(uint8_t col, uint8_t row, uint8_t layer);
void s_put_symbol(uint8_t symbol, uint8_t color, uint8_t layer);

#endif /* SCREEN_H */
