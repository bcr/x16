#include <stdio.h>
#include <stdint.h>

static void output_hex_digit(uint8_t digit)
{
    putchar(((digit < 10) ? '0' : ('a' - 10)) + digit);
}

static uint8_t is_scary_char(uint8_t the_char)
{
    return (the_char < 0x20) || ((the_char >= 0x80) && (the_char <= 0x9f));
}

static void output_char_map()
{
    uint8_t start_char = 0x00;
    uint8_t end_char = 0xfe;
    // uint8_t end_char = 0x7f;
    uint8_t current_char;
    uint8_t is_first_time = 1;

    puts("  \xdd\x30 1 2 3 4 5 6 7 8 9 a b c d e f");
    puts("\xc0\xc0\xdb\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0\xc0");

    for (current_char = start_char;current_char <= end_char;++current_char)
    {
        if ((current_char % 0x10) == 0)
        {
            if (!is_first_time)
            {
                putchar('\n');
                puts("  \xdd");
            }
            else
                is_first_time = 0;
            output_hex_digit(current_char / 0x10);
            putchar('0');
            putchar('\xdd');
        }
        putchar(is_scary_char(current_char) ? ' ' : current_char);
        putchar(' ');
    }
}

void main(void)
{
    output_char_map();
}
