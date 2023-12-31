#include <stdint.h>

#include <cbm.h>

#include "screen.h"

// https://github.com/mwiedmann/cx16CodingInC/blob/main/Chapter07-MapBase/main.c
// https://discord.com/channels/547559626024157184/629903553028161566/1129061635336708136

#define STRIDE 128
#define MAPBASE_TILE_COUNT STRIDE*64

// I left layer1MapBaseAddr at 0x1b000 so that printf() will continue to work.
// The tidy thing to do would be to put the two buffers next to each other.
// static const unsigned long layer1MapBaseAddr = 0x13000;
static const unsigned long layer1MapBaseAddr = 0x1b000;
static const unsigned long layer0MapBaseAddr = 0x13000 + (MAPBASE_TILE_COUNT * 2);
// static const unsigned long layer0MapBaseAddr = 0x1F000;

static void vera_set_start_line(uint8_t col, uint8_t row, uint8_t layer)
{
    unsigned long address = ((layer == 0) ? layer0MapBaseAddr : layer1MapBaseAddr) + (row * STRIDE * 2) + (col * 2);

    VERA.address = address;
    VERA.address_hi = address>>16;
    VERA.address_hi |= 0b10000;
}

void s_init(void)
{
    VERA.layer1.mapbase = layer1MapBaseAddr >> 9;
    VERA.layer0.mapbase = layer0MapBaseAddr >> 9;

    VERA.layer0.tilebase = VERA.layer1.tilebase;
    VERA.layer0.config = VERA.layer1.config;

    // I added the clear calls and switch the layers after that so it doesn't
    // look janky at startup
    s_clear(0, 0);
    s_clear(0, 1);

    vera_layer_enable(0b11);    // Turn on layer 0 and layer 1
}

void s_set_position(uint8_t col, uint8_t row, uint8_t layer)
{
    vera_set_start_line(col, row, layer);
}

void s_put_symbol(uint8_t symbol, uint8_t color)
{
    VERA.data0 = symbol;
    VERA.data0 = color;
}

void s_clear(uint8_t color, uint8_t layer)
{
    uint_least16_t i;

    s_set_position(0, 0, layer);

    for (i=0; i<MAPBASE_TILE_COUNT; i++)
    {
        VERA.data0 = 32; // space
        VERA.data0 = color;
    }
}

void s_scroll(int8_t cols, int8_t rows, uint8_t layer)
{
    if (layer == 0)
    {
        VERA.layer0.hscroll = cols * 8;
        VERA.layer0.vscroll = rows * 8;
    }
    else
    {
        VERA.layer1.hscroll = cols * 8;
        VERA.layer1.vscroll = rows * 8;
    }
}
