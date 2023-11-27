#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MAX_CELL_COLUMN 63
#define MAX_CELL_ROW 254

#define COLUMN_WIDTH 9

#define SYMBOL_COMMERCIAL_AT 0x00
#define SYMBOL_SPACE 0x20
#define SYMBOL_LATIN_CAPITAL_LETTER_A 0x41
#define SYMBOL_LATIN_SMALL_LETTER_A 0x01
#define SYMBOL_FULL_STOP 0x2E
#define SYMBOL_DIGIT_ZERO 0x30
#define SYMBOL_GREATER_THAN_SIGN 0x3E

#define KEY_QUOTATION_MARK 0x22
#define KEY_DOLLAR_SIGN 0x24
#define KEY_ASTERISK 0x2A
#define KEY_PLUS_SIGN 0x2B
#define KEY_HYPHEN_MINUS 0x2D
#define KEY_SOLIDUS 0x2F
#define KEY_DIGIT_ZERO 0x30
#define KEY_DIGIT_NINE 0x39
#define KEY_GREATER_THAN_SIGN 0x3E
#define KEY_LATIN_SMALL_LETTER_A 0x41
#define KEY_LATIN_SMALL_LETTER_B 0x42
#define KEY_LATIN_SMALL_LETTER_D 0x44
#define KEY_LATIN_SMALL_LETTER_F 0x46
#define KEY_LATIN_SMALL_LETTER_L 0x4C
#define KEY_LATIN_SMALL_LETTER_R 0x52
#define KEY_LATIN_SMALL_LETTER_V 0x56
#define KEY_LATIN_SMALL_LETTER_Z 0x5A
#define KEY_LATIN_CAPITAL_LETTER_A 0xC1
#define KEY_LATIN_CAPITAL_LETTER_B 0xC2
#define KEY_LATIN_CAPITAL_LETTER_D 0xC4
#define KEY_LATIN_CAPITAL_LETTER_F 0xC6
#define KEY_LATIN_CAPITAL_LETTER_L 0xCC
#define KEY_LATIN_CAPITAL_LETTER_R 0xD2
#define KEY_LATIN_CAPITAL_LETTER_V 0xD6
#define KEY_LATIN_CAPITAL_LETTER_Z 0xDA

#define MAKE_CELLREF(COL, ROW) ((uint16_t) (((COL) << 8) | (ROW)))
#define CELLREF_GET_COL(CR) ((uint8_t) ((CR) >> 8))
#define CELLREF_GET_ROW(CR) ((uint8_t) ((CR) & 0x0FF))

uint8_t util_c_char_to_symbol(char c_char);
uint8_t util_convert_cstr_to_symbol(uint8_t* dest, uint8_t len, const char* c_str);
uint8_t util_symbols_to_cellref(const uint8_t* s, uint8_t len, uint8_t* consumed, uint16_t* cellref);

#endif /* UTIL_H */
