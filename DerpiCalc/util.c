#include "util.h"

uint8_t util_c_char_to_symbol(char c_char)
{
    uint8_t symbol;
    symbol = c_char;
    if ((symbol >= 'a') && (symbol <= 'z'))
    {
        symbol = symbol - 'a' + SYMBOL_LATIN_SMALL_LETTER_A;
    }
    else if ((symbol >= 'A') && (symbol <= 'Z'))
    {
        symbol = symbol - 'A' + SYMBOL_LATIN_CAPITAL_LETTER_A;
    }
    else if ((symbol >= ' ') && (symbol <= '?'))
    {
    }
    else if (symbol == '[')
    {
        symbol = 27;
    }
    else if (symbol == ']')
    {
        symbol = 29;
    }
    else if (symbol == '@')
    {
        symbol = 0;
    }
    else
    {
        symbol = '?';
    }

    return symbol;
}

uint8_t util_convert_cstr_to_symbol(uint8_t* dest, uint8_t len, const char* c_str)
{
    uint8_t copied = 0;

    while ((copied < len) && (*c_str))
    {
        dest[copied] = util_c_char_to_symbol(*c_str);
        ++c_str;
        ++copied;
    }
    return copied;
}
