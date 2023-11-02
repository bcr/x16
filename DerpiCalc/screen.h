/// @file screen.h

#ifndef SCREEN_H
#define SCREEN_H

/// @brief Width of the visible screen in characters.
#define WIDTH_CHARS 80
/// @brief Height of the visible screen in characters.
#define HEIGHT_CHARS 60

#define MAKECOLOR(FG, BG) ((FG) | (BG) << 4)

/// @brief Initialize the screen routines.
void s_init(void);
void s_clear(uint8_t color, uint8_t layer);
void s_set_position(uint8_t col, uint8_t row, uint8_t layer);
void s_put_symbol(uint8_t symbol, uint8_t color);
void s_scroll(int8_t cols, int8_t rows, uint8_t layer);

#endif /* SCREEN_H */
