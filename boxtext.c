#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define UPPER_LEFT      '\xb0'
#define UPPER_RIGHT     '\xae'
#define HORIZONTAL_LINE '\xc0'
#define VERTICAL_LINE   '\xdd'
#define LOWER_LEFT      '\xad'
#define LOWER_RIGHT     '\xbd'
#define EOL             '\n'

static void puts_no_eol(const char* text)
{
    const char* ptr;
    for (ptr = text;*ptr;++ptr)
    {
        putchar(*ptr);
    }
}

static void putchar_dup(char the_char, uint8_t times)
{
    uint8_t i;

    for (i = 0;i < times;++i)
    {
        putchar(the_char);
    }
}

static void draw_boxed_text(const char* text)
{
    uint8_t text_len = (uint8_t) strlen(text);

    putchar(UPPER_LEFT);
    putchar_dup(HORIZONTAL_LINE, text_len);
    putchar(UPPER_RIGHT);
    putchar(EOL);

    putchar(VERTICAL_LINE);
    puts_no_eol(text);
    putchar(VERTICAL_LINE);
    putchar(EOL);

    putchar(LOWER_LEFT);
    putchar_dup(HORIZONTAL_LINE, text_len);
    putchar(LOWER_RIGHT);
    putchar(EOL);
}

void main(void)
{
    draw_boxed_text("commander x16 yo");
}
