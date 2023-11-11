#include <string.h>

#include "cell.h"
#include "dc_math.h"
#include "util.h"

#include "expr.h"

#define MAX_OPERATORS 10
#define MAX_OPERANDS 10

typedef void (*operator_func)(const struct number_t* a, const struct number_t* b, struct number_t* result);

static operator_func get_operator_func(uint8_t operator, uint8_t* rc)
{
    *rc = EVALUATE_OK;
    switch (operator)
    {
        case '/':
            return m_divide;
        case '*':
            return m_multiply;
        case '+':
            return m_add;
        case '-':
            return m_subtract;
        case '(':
            return NULL;
        default:
            *rc = EVALUATE_UNKNOWN_OPERATOR;
            return NULL;
    }
}

#define PARSING_COLUMN_1 0
#define PARSING_COLUMN_2 1
#define PARSING_ROW 2

static uint8_t m_symbols_to_cellref(const uint8_t* s, uint8_t len, uint8_t* consumed, uint16_t* cellref)
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

static uint8_t e_symbols_to_at(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t rc = EVALUATE_BAD_AT_SEQUENCE;

    *consumed = 0;
    if (len >= 3)
    {
        if (((expression[1] - SYMBOL_LATIN_CAPITAL_LETTER_A) == ('P' - 'A')) &&
            ((expression[2] - SYMBOL_LATIN_CAPITAL_LETTER_A) == ('I' - 'A')))
        {
            // !!! TODO Should constants be a simpler process?
            e_evaluate("3.1415926536", 12, result);
            *consumed = 3;
            rc = EVALUATE_OK;
        }
    }

    return rc;
}

static uint8_t e_symbols_to_number(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t rc;
    uint16_t cellref;

    if ((expression[0] >= SYMBOL_LATIN_CAPITAL_LETTER_A) && (expression[0] <= (SYMBOL_LATIN_CAPITAL_LETTER_A + 25)))
    {
        rc = m_symbols_to_cellref(expression, len, consumed, &cellref);
        if (rc != EVALUATE_OK)
            return rc;
        rc = c_get_cell_number(CELLREF_GET_COL(cellref), CELLREF_GET_ROW(cellref), result);
        if (rc != EVALUATE_OK)
            return rc;
    }
    else if (expression[0] == SYMBOL_COMMERCIAL_AT)
    {
        rc = e_symbols_to_at(expression, len, result, consumed);
        if (rc != EVALUATE_OK)
            return rc;
    }
    else
    {
        m_symbols_to_number(expression, len, result, consumed);
        rc = EVALUATE_OK;
    }

    return rc;
}

uint8_t e_evaluate(const uint8_t* expression, uint8_t len, struct number_t* result)
{
    uint8_t operators[MAX_OPERATORS];
    int8_t current_operator = 0;
    struct number_t operands[MAX_OPERANDS];
    int8_t current_operand = 0;
    uint8_t index = 0;
    uint8_t this_symbol;
    uint8_t consumed;
    uint8_t rc;
    operator_func operator;

    while (index < len)
    {
        if ((current_operand >= MAX_OPERANDS) || (current_operator >= MAX_OPERATORS))
        {
            return EVALUATE_TOO_COMPLEX;
        }

        this_symbol = expression[index];
        consumed = 1;
        if ((this_symbol == '(') || (this_symbol == '+') || (this_symbol == '*') || (this_symbol == '/')  || (this_symbol == '-'))
        {
            operators[current_operator] = this_symbol;
            ++current_operator;
        }
        else if (this_symbol == ')')
        {
            while (current_operator > 0)
            {
                operator = get_operator_func(operators[current_operator - 1], &rc);
                if (rc != EVALUATE_OK)
                    return rc;
                if (operators[current_operator - 1] == '(')
                {
                    --current_operator;
                    break;
                }
                if (current_operand < 2)
                {
                    return EVALUATE_GENERAL_ERROR;
                }

                if (operator != NULL)
                    operator(&(operands[current_operand - 2]), &(operands[current_operand - 1]), &(operands[current_operand - 2]));
                --current_operator;
                --current_operand;
            }
        }
        else
        {
            rc = e_symbols_to_number(expression + index, len - index, &(operands[current_operand]), &consumed);
            if (rc != EVALUATE_OK)
                return rc;
            ++current_operand;
            if ((current_operand > 1) && (current_operator > 0) && (operators[current_operator - 1] != '('))
            {
                operator = get_operator_func(operators[current_operator - 1], &rc);
                if (rc != EVALUATE_OK)
                    return rc;
                if (operator != NULL)
                    operator(&(operands[current_operand - 2]), &(operands[current_operand - 1]), &(operands[current_operand - 2]));
                --current_operand;
                --current_operator;
            }
        }
        index += consumed;
    }

    while (current_operator > 0)
    {
        operator = get_operator_func(operators[current_operator - 1], &rc);
        if (rc != EVALUATE_OK)
            return rc;
        if (operators[current_operator - 1] == '(')
        {
            return EVALUATE_UNBALANCED_PARENS;
        }
        if (current_operand < 2)
        {
            return EVALUATE_GENERAL_ERROR;
        }

        if (operator != NULL)
            operator(&(operands[current_operand - 2]), &(operands[current_operand - 1]), &(operands[current_operand - 2]));
        --current_operator;
        --current_operand;
    }

    memcpy(result, &operands[0], sizeof(struct number_t));
    return EVALUATE_OK;
}
