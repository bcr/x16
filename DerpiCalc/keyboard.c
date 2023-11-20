#include <cbm.h>

#include "keyboard.h"

void kb_init(void)
{
}

uint8_t kb_getch(void)
{
    uint8_t keycode;

    while(1) {
        keycode = cbm_k_getin();
        if (keycode) {
            return keycode;
        }
    }
}
