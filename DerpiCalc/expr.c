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
        default:
            *rc = EVALUATE_UNKNOWN_OPERATOR;
            return NULL;
    }
}

struct comma_iter
{
    const uint8_t* buffer;
    uint8_t buffer_len;
    uint8_t first;

    uint8_t pos;
    uint8_t len;
    uint8_t last;
};

static void comma_next(struct comma_iter* iter)
{
    uint8_t i;
    uint8_t paren_count;

    if (!iter->first)
    {
        iter->pos += (iter->len + 1);
    }
    else
        iter->first = 0;

    paren_count = 0;

    for (i = iter->pos;i < iter->buffer_len;++i)
    {
        if (iter->buffer[i] == '(')
            ++paren_count;
        else if (iter->buffer[i] == ')')
            --paren_count;
        else if ((iter->buffer[i] == ',') && (paren_count == 0))
        {
            iter->len = i - iter->pos;
            return;
        }
    }

    iter->last = 1;
    iter->len = iter->buffer_len - iter->pos;
}

static void comma_start(struct comma_iter* iter, const uint8_t* buffer, uint8_t len)
{
    iter->buffer = buffer;
    iter->buffer_len = len;
    iter->first = 1;
    iter->pos = 0;
    iter->len = 0;
    iter->last = 0;
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

// These are all zero-length @ functions and ignore their first parameter
#pragma warn (unused-param, push, off)
static uint8_t handle_pi(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return e_evaluate("3.1415926536", 12, result);
}

static uint8_t handle_na(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    result->type = NUMBER_TYPE_NA;
    return EVALUATE_OK;
}

static uint8_t handle_error(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    result->type = NUMBER_TYPE_ERROR;
    return EVALUATE_OK;
}
#pragma warn (unused-param, pop)

static uint8_t handle_abs(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc;
    rc = e_evaluate(buffer, len, result);
    if (rc != EVALUATE_OK)
        return rc;
    m_abs(result, result);
    return EVALUATE_OK;
}

static uint8_t handle_sum(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc = EVALUATE_OK;
    struct comma_iter c_iter;
    struct number_t eval_result;

    m_int_to_number(0, result);

    comma_start(&c_iter, buffer, len);
    do
    {
        comma_next(&c_iter);
        rc = e_evaluate(c_iter.buffer + c_iter.pos, c_iter.len, &eval_result);
        if (rc != EVALUATE_OK)
            break;

        m_add(result, &eval_result, result);
    } while (!c_iter.last);

    return rc;
}

struct at_func
{
    const char* name;
    uint8_t (*handler)(const uint8_t* buffer, uint8_t len, struct number_t* result);
};

static const struct at_func zero_len_at_funcs[] = {
    { "PI", handle_pi },
    { "ERROR", handle_error },
    { "NA", handle_na },

    { NULL, NULL }
    };

static const struct at_func nonzero_len_at_funcs[] = {
    { "ABS", handle_abs },
    { "SUM", handle_sum },

    { NULL, NULL }
    };

static const struct at_func* find_symbol(const struct at_func* functions, const uint8_t* expression, uint8_t len, uint8_t* consumed)
{
    uint8_t i;
    uint8_t pos;

    // Skip the atsign
    ++expression;
    --len;

    for (i = 0;functions[i].name != NULL;++i)
    {
        for (pos = 0;
            (functions[i].name[pos]) && (pos < len) &&
            ((functions[i].name[pos] - 'A') == (expression[pos] - SYMBOL_LATIN_CAPITAL_LETTER_A));
            ++pos)
            ;
        if (functions[i].name[pos] == 0)
            break;
    }

    if (functions[i].name != NULL)
        *consumed = (strlen(functions[i].name) + 1);
    return &(functions[i]);
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

static uint8_t e_symbols_to_at(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed)
{
    uint8_t rc = EVALUATE_OK;
    const struct at_func* atfunc;
    uint8_t closing_paren_index;

    *consumed = 0;
    atfunc = find_symbol(zero_len_at_funcs, expression, len, consumed);
    if (atfunc->handler != NULL)
    {
        return atfunc->handler(NULL, 0, result);
    }

    atfunc = find_symbol(nonzero_len_at_funcs, expression, len, consumed);
    if (atfunc->handler != NULL)
    {
        rc = find_closing_paren(expression + *consumed, len - *consumed, &closing_paren_index);
        if (rc != EVALUATE_OK)
            return rc;
        // Start past the open paren, end before the close paren
        rc = atfunc->handler(expression + *consumed + 1, closing_paren_index - 1, result);
        if (rc != EVALUATE_OK)
            return rc;
        *consumed += (closing_paren_index + 1);

        return rc;
    }
    rc = EVALUATE_BAD_AT_SEQUENCE;
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
