#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define IS_ARROW_KEY(K) (((K) == CH_CURS_LEFT) || ((K) == CH_CURS_RIGHT) || ((K) == CH_CURS_UP) || ((K) == CH_CURS_DOWN))

void kb_init(void);
uint8_t kb_getch(void);

#endif /* KEYBOARD_H */
