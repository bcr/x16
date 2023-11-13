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

#define ATFUNC_UNKNOWN 0
#define ATFUNC_PI 1

struct at_func
{
    const char* name;
    uint8_t value;
};

#define ATFUNC_ENTRY(X) { #X, ATFUNC_##X },
#define ATFUNC_LAST_ENTRY { "", ATFUNC_UNKNOWN }

static const struct at_func zero_len_at_funcs[] = {
    ATFUNC_ENTRY(PI)

    ATFUNC_LAST_ENTRY
    };

static uint8_t find_symbol(const struct at_func* functions, const uint8_t* expression, uint8_t len, uint8_t* consumed)
{
    uint8_t i;
    uint8_t pos;

    // Skip the atsign
    ++expression;
    --len;

    for (i = 0;functions[i].value != ATFUNC_UNKNOWN;++i)
    {
        for (pos = 0;
            (functions[i].name[pos]) && (pos < len) &&
            ((functions[i].name[pos] - 'A') == (expression[pos] - SYMBOL_LATIN_CAPITAL_LETTER_A));
            ++pos)
            ;
        if (functions[i].name[pos] == 0)
            break;
    }

    *consumed = (strlen(functions[i].name) + 1);
    return functions[i].value;
}

static uint8_t e_symbols_to_at(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t rc = EVALUATE_OK;
    uint8_t atfunc;

    *consumed = 0;
    atfunc = find_symbol(zero_len_at_funcs, expression, len, consumed);
    switch (atfunc)
    {
        case ATFUNC_PI:
            // !!! TODO Should constants be a simpler process?
            e_evaluate("3.1415926536", 12, result);
            break;
        default:
            rc = EVALUATE_BAD_AT_SEQUENCE;
            break;
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

static uint8_t find_closing_paren(const uint8_t* expression, uint8_t len, uint8_t* closing_paren_index)
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
            if (operator != NULL)
                operator(&operand_a, &operand_b, &operand_a);
            current_operator = 0;
            perform_operation = 0;
        }
    }

    memcpy(result, &operand_a, sizeof(struct number_t));
    return EVALUATE_OK;
}
