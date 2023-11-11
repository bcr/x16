#ifndef UTIL_H
#define UTIL_H

#define MAX_CELL_COLUMN 63
#define MAX_CELL_ROW 254

#define COLUMN_WIDTH 9

#define SYMBOL_SPACE 0x20
#define SYMBOL_LATIN_CAPITAL_LETTER_A 0x41
#define SYMBOL_LATIN_SMALL_LETTER_A 0x01
#define SYMBOL_DIGIT_ZERO 0x30

#define MAKE_CELLREF(COL, ROW) ((uint16_t) (((COL) << 8) | (ROW)))
#define CELLREF_GET_COL(CR) ((uint8_t) ((CR) >> 8))
#define CELLREF_GET_ROW(CR) ((uint8_t) ((CR) & 0x0FF))

#endif /* UTIL_H */
