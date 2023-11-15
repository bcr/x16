#ifndef EXPR_INTERNAL_H
#define EXPR_INTERNAL_H

uint8_t e_symbols_to_at(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed);
uint8_t m_symbols_to_cellref(const uint8_t* s, uint8_t len, uint8_t* consumed, uint16_t* cellref);
uint8_t find_closing_paren(const uint8_t* expression, uint8_t len, uint8_t* closing_paren_index);

#endif /* EXPR_INTERNAL_H */
