#include <stdint.h>

#include <cbm.h>

// https://github.com/mwiedmann/cx16CodingInC/blob/main/Chapter07-MapBase/main.c
// https://discord.com/channels/547559626024157184/629903553028161566/1129061635336708136

#define STRIDE 128
#define MAPBASE_TILE_COUNT STRIDE*64

#define MAKECOLOR(FG, BG) ((FG) | (BG) << 4)
#define NORMAL_COLOR MAKECOLOR(COLOR_WHITE, COLOR_BLUE)

static const unsigned long layer1MapBaseAddr = 0x1b000;

static uint8_t current_row = 0;

static void vera_set_start_line(uint8_t col, uint8_t row)
{
    unsigned long address = layer1MapBaseAddr + (row * STRIDE * 2) + (col * 2);

    VERA.address = address;
    VERA.address_hi = address>>16;
    VERA.address_hi |= 0b10000;
}

void s_set_position(uint8_t col, uint8_t row)
{
    vera_set_start_line(col, row);
}

void s_clear(uint8_t color)
{
    uint_least16_t i;

    s_set_position(0, 0);

    for (i=0; i<MAPBASE_TILE_COUNT; i++)
    {
        VERA.data0 = 32; // space
        VERA.data0 = color;
    }
}

void s_init(void)
{
    s_clear(NORMAL_COLOR);
    s_set_position(0, current_row);
}

void s_put_symbol(uint8_t symbol, uint8_t color)
{
    VERA.data0 = symbol;
    VERA.data0 = color;
}

static void s_putsymbol(uint8_t c)
{
    s_put_symbol(c, NORMAL_COLOR);
}

static void s_putchar(uint8_t c)
{
    s_put_symbol(c, NORMAL_COLOR);
}

static void s_puts(const uint8_t* s)
{
    for (;*s;++s)
        s_putchar(*s);
    ++current_row;
    s_set_position(0, current_row);
}

static void output_hex_digit(uint8_t digit)
{
    s_putchar(((digit < 10) ? '0' : (-9)) + digit);
}

static void output_char_map()
{
    uint8_t start_char = 0x00;
    uint8_t end_char = 0xfe;
    // uint8_t end_char = 0x7f;
    uint8_t current_char;
    uint8_t is_first_time = 1;

    s_puts("  \x5d" "0 1 2 3 4 5 6 7 8 9 \x01 \x02 \x03 \x04 \x05 \x06");
    s_puts("\x40\x40\x5b\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40\x40");

    for (current_char = start_char;current_char <= end_char;++current_char)
    {
        if ((current_char % 0x10) == 0)
        {
            if (!is_first_time)
            {
                s_puts("");
                s_puts("  \x5d");
            }
            else
                is_first_time = 0;
            output_hex_digit(current_char / 0x10);
            s_putchar('0');
            s_putchar('\x5d');
        }
        s_putsymbol(current_char);
        s_putchar(' ');
    }
}

void main(void)
{
    s_init();
    output_char_map();
}
