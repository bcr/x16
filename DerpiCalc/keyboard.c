#include "keyboard.h"

static uint8_t keycode;

void kb_init(void)
{
}

uint8_t kb_getch(void)
{
    while(1) {
#if __CC65__
        asm("jsr $FFE4");
        asm("sta %v", keycode);
#else
    asm volatile ("JSR $FFE4" : "=a"(keycode)::"a","c","v","x","y");
#endif /* __CC65__ */

        if (keycode) {
            return keycode;
        }
    }
}
