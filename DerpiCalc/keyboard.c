#include <stdio.h>

#include "keyboard.h"

static uint8_t keycode;

void kb_init(void)
{
}

uint8_t kb_getch(void)
{
    while(1) {
        asm("jsr $FFE4");
        asm("sta %v", keycode);

        if (keycode) {
            return keycode;
        }
    }
}
