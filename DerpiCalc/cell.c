#include <stddef.h>

#include "cell.h"

#define SYMBOL_SPACE 0x20
#define SYMBOL_LATIN_CAPITAL_LETTER_A 0x41
#define SYMBOL_LATIN_SMALL_LETTER_A 0x01
#define SYMBOL_DIGIT_ZERO 0x30

cell_ctx c_init(void)
{
    return 1;
}

static uint8_t cell_value[] = "         ";

const uint8_t* c_get_cell_value(cell_ctx ctx, uint8_t col, uint8_t row)
{
    uint8_t index = 0;
    uint8_t one_based_row = row + 1;

    cell_value[index++] = (col < 26 ? SYMBOL_SPACE : (col / 26 - 1) + SYMBOL_LATIN_CAPITAL_LETTER_A);
    cell_value[index++] = (col % 26 + SYMBOL_LATIN_CAPITAL_LETTER_A);
    cell_value[index++] = (one_based_row < 100 ? SYMBOL_SPACE : one_based_row / 100 + SYMBOL_DIGIT_ZERO);
    cell_value[index++] = (one_based_row < 10 ? SYMBOL_SPACE : one_based_row / 10 % 10 + SYMBOL_DIGIT_ZERO);
    cell_value[index++] = (one_based_row % 10 + SYMBOL_DIGIT_ZERO);
    cell_value[index++] = SYMBOL_SPACE;
    cell_value[index++] = col;
    cell_value[index++] = row;
    return cell_value;
}
