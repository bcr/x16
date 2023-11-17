#include <string.h>

#include "cell.h"
#include "dc_math.h"
#include "util.h"

#include "expr.h"
#include "expr_internal.h"

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
        default:
            *rc = EVALUATE_UNKNOWN_OPERATOR;
            return NULL;
    }
}

uint8_t find_closing_paren(const uint8_t* expression, uint8_t len, uint8_t* closing_paren_index)
{
    uint8_t i;
    uint8_t paren_count = 0;

    for (i = 0;i < len;++i)
    {
        if (expression[i] == '(')
            ++paren_count;
        else if (expression[i] == ')')
            --paren_count;
        if (paren_count == 0)
            break;
    }

    if (paren_count > 0)
        return EVALUATE_UNBALANCED_PARENS;

    *closing_paren_index = i;
    return EVALUATE_OK;
}

static uint8_t e_symbols_to_number(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t rc;
    uint16_t cellref;

    if ((expression[0] >= SYMBOL_LATIN_CAPITAL_LETTER_A) && (expression[0] <= (SYMBOL_LATIN_CAPITAL_LETTER_A + 25)))
    {
        rc = util_symbols_to_cellref(expression, len, consumed, &cellref);
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
    int8_t current_operator = 0;
    struct number_t operand_a;
    struct number_t operand_b;
    uint8_t index = 0;
    uint8_t this_symbol;
    uint8_t consumed;
    uint8_t rc;
    operator_func operator;
    uint8_t closing_paren_index;
    uint8_t perform_operation = 0;

    while (index < len)
    {
        this_symbol = expression[index];
        if (this_symbol == '(')
        {
            rc = find_closing_paren(expression + index, len - index, &closing_paren_index);
            if (rc != EVALUATE_OK)
                return rc;

            // Start past the open paren, end before the close paren
            rc = e_evaluate(expression + index + 1, closing_paren_index - 1, (current_operator) ? &operand_b : &operand_a);
            if (rc != EVALUATE_OK)
                return rc;
            if (current_operator)
                perform_operation = 1;
            index += (closing_paren_index + 1);
        }
        else if ((this_symbol == '+') || (this_symbol == '*') || (this_symbol == '/')  || (this_symbol == '-'))
        {
            current_operator = this_symbol;
            index += 1;
        }
        else
        {
            rc = e_symbols_to_number(expression + index, len - index, (current_operator) ? &operand_b : &operand_a, &consumed);
            if (rc != EVALUATE_OK)
                return rc;
            if (current_operator)
                perform_operation = 1;
            index += consumed;
        }

        if (perform_operation)
        {
            operator = get_operator_func(current_operator, &rc);
            if (rc != EVALUATE_OK)
                return rc;
            if ((operand_a.type == NUMBER_TYPE_NORMAL) && (operand_b.type == NUMBER_TYPE_NORMAL))
                operator(&operand_a, &operand_b, &operand_a);
            else if ((operand_a.type == NUMBER_TYPE_ERROR) || (operand_b.type == NUMBER_TYPE_ERROR))
                operand_a.type = NUMBER_TYPE_ERROR;
            else if ((operand_a.type == NUMBER_TYPE_NA) || (operand_b.type == NUMBER_TYPE_NA))
                operand_a.type = NUMBER_TYPE_NA;
            current_operator = 0;
            perform_operation = 0;
        }
    }

    memcpy(result, &operand_a, sizeof(struct number_t));
    return EVALUATE_OK;
}
