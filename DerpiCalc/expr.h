#ifndef EXPR_H
#define EXPR_H

#define EVALUATE_OK 0
#define EVALUATE_UNKNOWN_OPERATOR 1
#define EVALUATE_UNBALANCED_PARENS 2
#define EVALUATE_TOO_COMPLEX 3
#define EVALUATE_GENERAL_ERROR 4
#define EVALUATE_BAD_CELL_REFERENCE 5
#define EVALUATE_BAD_AT_SEQUENCE 6
#define EVALUATE_BAD_RANGE 7

uint8_t e_evaluate(const uint8_t* expression, uint8_t len, struct number_t* result);

#endif /* EXPR_H */
