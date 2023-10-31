#include <stdint.h>

#include <cx16.h>

#include "screen.h"

// https://github.com/mwiedmann/cx16CodingInC/blob/main/Chapter07-MapBase/main.c
// https://discord.com/channels/547559626024157184/629903553028161566/1129061635336708136

#define STRIDE 128
#define MAPBASE_TILE_COUNT STRIDE*64

static const unsigned long mapBaseAddr = 0x1B000;

static void vera_set_start_line(uint8_t col, uint8_t row)
{
    unsigned long address = mapBaseAddr + (row * STRIDE * 2) + (col * 2);

    VERA.address = address;
    VERA.address_hi = address>>16;
    VERA.address_hi |= 0b10000;
}

void s_init(void)
{
}

void s_set_position(uint8_t col, uint8_t row)
{
    vera_set_start_line(col, row);
}

void s_put_symbol(uint8_t symbol, uint8_t color)
{
    VERA.data0 = symbol;
    VERA.data0 = color;
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
