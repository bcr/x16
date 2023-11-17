#include <stdint.h>

#include "expr.h"

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

#define PARSING_COLUMN_1 0
#define PARSING_COLUMN_2 1
#define PARSING_ROW 2

uint8_t util_symbols_to_cellref(const uint8_t* s, uint8_t len, uint8_t* consumed, uint16_t* cellref)
{
    uint8_t col = 0;
    uint8_t row = 0;
    uint8_t index = 0;
    uint8_t this_symbol;
    uint8_t state = PARSING_COLUMN_1;
    uint8_t spin_again;
    uint8_t done = 0;

    while (index < len)
    {
        this_symbol = s[index];
        spin_again = 0;
        switch (state)
        {
            case PARSING_COLUMN_1:
                if ((this_symbol >= SYMBOL_LATIN_CAPITAL_LETTER_A) && (this_symbol <= (SYMBOL_LATIN_CAPITAL_LETTER_A + 25)))
                {
                    col = this_symbol - SYMBOL_LATIN_CAPITAL_LETTER_A;
                    state = PARSING_COLUMN_2;
                }
                else
                {
                    return EVALUATE_BAD_CELL_REFERENCE;
                }
                break;
            case PARSING_COLUMN_2:
                if ((this_symbol >= SYMBOL_LATIN_CAPITAL_LETTER_A) && (this_symbol <= (SYMBOL_LATIN_CAPITAL_LETTER_A + 25)))
                    col = ((col+1) * 26) + (this_symbol - SYMBOL_LATIN_CAPITAL_LETTER_A);
                else
                    spin_again = 1;
                state = PARSING_ROW;
                break;
            case PARSING_ROW:
                if ((this_symbol >= SYMBOL_DIGIT_ZERO) && (this_symbol <= (SYMBOL_DIGIT_ZERO + 9)))
                {
                    row *= 10;
                    row += (this_symbol - SYMBOL_DIGIT_ZERO);
                }
                else
                    done = 1;
                break;
        }
        if (done)
            break;
        if (!spin_again)
            ++index;
    }

    row -= 1;
    *consumed = index;
    *cellref = MAKE_CELLREF(col, row);
    return EVALUATE_OK;
}
