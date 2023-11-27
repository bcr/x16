#include <string.h>

#include <stdint.h>

#include "expr.h"
#include "expr_internal.h"
#include "util.h"
#include "dc_math.h"
#include "cell.h"

struct range_iter
{
    uint8_t col1;
    uint8_t col2;
    uint8_t row1;
    uint8_t row2;
    uint8_t first;

    int8_t direction;

    // Permissible for caller to inspect
    uint8_t current_col;
    uint8_t current_row;
    uint8_t last;
};

static uint8_t range_start(struct range_iter *iter, uint16_t start, uint16_t end)
{
    iter->col1 = CELLREF_GET_COL(start);
    iter->row1 = CELLREF_GET_ROW(start);
    iter->col2 = CELLREF_GET_COL(end);
    iter->row2 = CELLREF_GET_ROW(end);

    if ((iter->col1 != iter->col2) && (iter->row1 != iter->row2))
        return EVALUATE_BAD_RANGE;
    else if (iter->col1 != iter->col2)
        iter->direction = (iter->col1 < iter->col2) ? +1 : -1;
    else
        iter->direction = (iter->row1 < iter->row2) ? +1 : -1;

    iter->current_col = iter->col1;
    iter->current_row = iter->row1;
    iter->last = (iter->current_col == iter->col2) && (iter->current_row == iter->row2);
    iter->first = 1;

    return EVALUATE_OK;
}

static void range_next(struct range_iter *iter)
{
    if (iter->first)
    {
        iter->first = 0;
        return;
    }

    if (iter->col1 != iter->col2)
        iter->current_col += iter->direction;
    else
        iter->current_row += iter->direction;

    iter->last = (iter->current_col == iter->col2) && (iter->current_row == iter->row2);

    return;
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

// These are all zero-length @ functions and ignore their first parameter
#pragma warn (unused-param, push, off)
static uint8_t handle_pi(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return e_evaluate((const uint8_t*) "3.1415926536", 12, result);
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

static uint8_t handle_single(const uint8_t* buffer, uint8_t len, struct number_t* result, void (*func)(const struct number_t* a, struct number_t* result))
{
    uint8_t rc;
    rc = e_evaluate(buffer, len, result);
    if (rc != EVALUATE_OK)
        return rc;
    func(result, result);
    return EVALUATE_OK;
}

static uint8_t handle_abs(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_abs);
}

static uint8_t maybe_parse_range(const uint8_t* buffer, uint8_t len, struct range_iter* iter)
{
    uint8_t rc;
    uint8_t total_consumed = 0;
    uint8_t consumed;
    uint8_t i;
    uint16_t cellref_start;
    uint16_t cellref_end;

    rc = util_symbols_to_cellref(buffer + total_consumed, len - total_consumed, &consumed, &cellref_start);
    if (rc != EVALUATE_OK)
        return EVALUATE_BAD_RANGE;
    total_consumed += consumed;

    if ((len - total_consumed) > 3)
    {
        for (i = 0;i < 3;++i)
            if (buffer[total_consumed + i] != '.')
                return EVALUATE_BAD_RANGE;

        total_consumed += 3;
    }
    else
    {
        return EVALUATE_BAD_RANGE;
    }

    rc = util_symbols_to_cellref(buffer + total_consumed, len - total_consumed, &consumed, &cellref_end);
    if (rc != EVALUATE_OK)
        return EVALUATE_BAD_RANGE;
    total_consumed += consumed;

    return range_start(iter, cellref_start, cellref_end);
}

typedef uint8_t (*number_func)(void* state, const struct number_t* number);

static uint8_t number_iter(const uint8_t* buffer, uint8_t len, void* state, number_func func)
{
    uint8_t rc = EVALUATE_OK;
    struct comma_iter c_iter;
    struct range_iter r_iter;
    struct number_t eval_result;

    comma_start(&c_iter, buffer, len);
    do
    {
        comma_next(&c_iter);
        rc = maybe_parse_range(c_iter.buffer + c_iter.pos, c_iter.len, &r_iter);
        if (rc == EVALUATE_OK)
        {
            do
            {
                range_next(&r_iter);
                rc = c_get_cell_number(r_iter.current_col, r_iter.current_row, &eval_result);
                if (rc != EVALUATE_OK)
                    return rc;
                rc = func(state, &eval_result);
                if (rc != EVALUATE_OK)
                    return rc;
            } while (!r_iter.last);
        }
        else if (rc == EVALUATE_BAD_RANGE)
        {
            rc = e_evaluate(c_iter.buffer + c_iter.pos, c_iter.len, &eval_result);
            if (rc != EVALUATE_OK)
                return rc;

            rc = func(state, &eval_result);
            if (rc != EVALUATE_OK)
                return rc;
        }
        else
            return rc;
    } while (!c_iter.last);

    return rc;
}

static uint8_t sum_func(void* state, const struct number_t* number)
{
    struct number_t* result = (struct number_t*) state;

    m_add(result, number, result);

    return EVALUATE_OK;
}

static uint8_t handle_sum(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    m_int_to_number(0, result);
    return number_iter(buffer, len, result, sum_func);
}

struct minmax_state
{
    uint8_t first;
    struct number_t* result;
};

static uint8_t max_func(void* state, const struct number_t* number)
{
    struct minmax_state* local_state = state;

    if ((local_state->first) || (m_compare(number, local_state->result) > 0))
        memcpy(local_state->result, number, sizeof(struct number_t));
    local_state->first = 0;

    return EVALUATE_OK;
}

static uint8_t handle_max(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    struct minmax_state state;
    state.first = 1;
    state.result = result;
    return number_iter(buffer, len, &state, max_func);
}

static uint8_t min_func(void* state, const struct number_t* number)
{
    struct minmax_state* local_state = state;

    if ((local_state->first) || (m_compare(number, local_state->result) < 0))
        memcpy(local_state->result, number, sizeof(struct number_t));
    local_state->first = 0;

    return EVALUATE_OK;
}

static uint8_t handle_min(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    struct minmax_state state;
    state.first = 1;
    state.result = result;
    return number_iter(buffer, len, &state, min_func);
}

struct average_state
{
    int16_t count;
    struct number_t* result;
};

static uint8_t average_func(void* state, const struct number_t* number)
{
    struct average_state* local_state = state;

    m_add(local_state->result, number, local_state->result);
    local_state->count++;

    return EVALUATE_OK;
}

static uint8_t handle_average(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    struct average_state state;
    struct number_t divisor;

    m_int_to_number(0, result);
    state.count = 0;
    state.result = result;

    number_iter(buffer, len, &state, average_func);
    m_int_to_number(state.count, &divisor);
    m_divide(state.result, &divisor, result);

    return EVALUATE_OK;
}

static uint8_t handle_sin(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_sin);
}

static uint8_t handle_cos(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_cos);
}

static uint8_t handle_tan(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_tan);
}

static uint8_t handle_atan(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_atan);
}

static uint8_t handle_ln(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_log);
}

static uint8_t handle_sqrt(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_sqr);
}

static uint8_t handle_exp(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_exp);
}

static uint8_t handle_int(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    return handle_single(buffer, len, result, m_int);
}

static uint8_t handle_log10(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc;
    struct number_t ln10;

    rc = e_evaluate(buffer, len, result);
    if (rc != EVALUATE_OK)
        return rc;

    // log10(x) = ln(x) / ln(10)
    m_int_to_number(10, &ln10); // !!! TODO: Constant, should we keep it?
    m_log(&ln10, &ln10);
    m_log(result, result);
    m_divide(result, &ln10, result);

    return EVALUATE_OK;
}

static uint8_t handle_asin(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc;
    struct number_t one;
    struct number_t x;

    rc = e_evaluate(buffer, len, result);
    if (rc != EVALUATE_OK)
        return rc;

    memcpy(&x, result, sizeof(struct number_t));
    m_int_to_number(1, &one); // !!! TODO: Constant?

    // If I can compute sin cos tan and atan can I compute asin?
    // arcsin(x) = arctan(x / √(1 - x^2))

    m_multiply(&x, &x, result); // result = x^2
    m_subtract(&one, result, result); // result = 1 - (x^2)
    m_sqr(result, result); // result = √(1 - x^2)
    m_divide(&x, result, result); // result = x / √(1 - x^2)
    m_atan(result, result); // result = arctan(x / √(1 - x^2))

    return EVALUATE_OK;
}

static uint8_t handle_acos(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc;
    struct number_t one;
    struct number_t x;

    rc = e_evaluate(buffer, len, result);
    if (rc != EVALUATE_OK)
        return rc;

    memcpy(&x, result, sizeof(struct number_t));
    m_int_to_number(1, &one); // !!! TODO: Constant?

    // arccos(x) = arctan(sqrt(1 - x²) / x)
    // arccos(-x) = π - arccos(x)

    m_multiply(&x, &x, result); // result = x^2
    m_subtract(&one, result, result); // result = 1 - (x^2)
    m_sqr(result, result); // result = √(1 - x^2)
    m_divide(result, &x, result); // result = √(1 - x^2) / x
    m_atan(result, result); // result = arctan(√(1 - x^2) / x)

    return EVALUATE_OK;
}

static uint8_t count_func(void* state, const struct number_t* number)
{
    uint8_t* result = state;

    // !!! TODO: What about blank cells.
    // What we really care about is counting nonblank values, not what the
    // actual values are
    *result += 1;

    return EVALUATE_OK;
}

static uint8_t handle_count(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc;
    uint8_t count = 0;

    rc = number_iter(buffer, len, &count, count_func);
    if (rc != EVALUATE_OK)
        return rc;
    m_int_to_number(count, result);
    return EVALUATE_OK;
}

static uint8_t handle_lookup(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc = EVALUATE_OK;
    uint8_t first = 1;
    struct comma_iter c_iter;
    struct range_iter r_iter;
    struct number_t eval_result;
    uint8_t col;
    uint8_t row;

    comma_start(&c_iter, buffer, len);

    comma_next(&c_iter);
    rc = e_evaluate(c_iter.buffer + c_iter.pos, c_iter.len, result);
    if (rc != EVALUATE_OK)
        return rc;

    if (c_iter.last)
        return EVALUATE_BAD_RANGE;

    comma_next(&c_iter);
    rc = maybe_parse_range(c_iter.buffer + c_iter.pos, c_iter.len, &r_iter);
    if (rc != EVALUATE_OK)
        return rc;

    do
    {
        range_next(&r_iter);
        if (r_iter.last && first)   // VisiCalc no like 1 cell lookup tables
            return EVALUATE_BAD_RANGE;

        rc = c_get_cell_number(r_iter.current_col, r_iter.current_row, &eval_result);
        if (rc != EVALUATE_OK)
            return rc;

        if ((m_compare(&eval_result, result) > 0) || (r_iter.last))
        {
            // If the first element is greater, NA is the answer
            if (first)
            {
                result->type = NUMBER_TYPE_NA;
                return EVALUATE_OK;
            }
            else
            {
                // Find the cell to the right or just below
                if (r_iter.col1 != r_iter.col2)
                    row++;
                else
                    col++;

                rc = c_get_cell_number(col, row, result);
                return rc;
            }
        }
        else
        {
            col = r_iter.current_col;
            row = r_iter.current_row;
            first = 0;
        }
    } while (!r_iter.last);

    return rc;
}

static uint8_t handle_npv(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    uint8_t rc = EVALUATE_OK;
    struct comma_iter c_iter;
    struct range_iter r_iter;
    struct number_t cash_flow;
    struct number_t discount;
    struct number_t current_discount;
    struct number_t this_npv;

    comma_start(&c_iter, buffer, len);

    comma_next(&c_iter);
    rc = e_evaluate(c_iter.buffer + c_iter.pos, c_iter.len, &discount);
    if (rc != EVALUATE_OK)
        return rc;

    m_int_to_number(1, &current_discount);
    m_add(&discount, &current_discount, &discount);
    m_int_to_number(0, result);

    if (c_iter.last)
        return EVALUATE_BAD_RANGE;

    comma_next(&c_iter);
    rc = maybe_parse_range(c_iter.buffer + c_iter.pos, c_iter.len, &r_iter);
    if (rc != EVALUATE_OK)
        return rc;

    do
    {
        range_next(&r_iter);

        m_multiply(&current_discount, &discount, &current_discount);

        rc = c_get_cell_number(r_iter.current_col, r_iter.current_row, &cash_flow);
        if (rc != EVALUATE_OK)
            return rc;

        m_divide(&cash_flow, &current_discount, &this_npv);
        m_add(result, &this_npv, result);
    } while (!r_iter.last);

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
    { "MAX", handle_max },
    { "MIN", handle_min },
    { "AVERAGE", handle_average },
    { "SIN", handle_sin },
    { "COS", handle_cos },
    { "TAN", handle_tan },
    { "ATAN", handle_atan },
    { "LN", handle_ln },
    { "SQRT", handle_sqrt },
    { "EXP", handle_exp },
    { "INT", handle_int },
    { "LOG10", handle_log10 },
    { "ASIN", handle_asin },
    { "ACOS", handle_acos },
    { "COUNT", handle_count },
    { "LOOKUP", handle_lookup },
    { "NPV", handle_npv },

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
            (util_c_char_to_symbol(functions[i].name[pos]) == expression[pos]);
            ++pos)
            ;
        if (functions[i].name[pos] == 0)
            break;
    }

    if (functions[i].name != NULL)
        *consumed = (strlen(functions[i].name) + 1);
    return &(functions[i]);
}

uint8_t e_symbols_to_at(const uint8_t* expression, uint8_t len, struct number_t* result, uint8_t* consumed)
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
