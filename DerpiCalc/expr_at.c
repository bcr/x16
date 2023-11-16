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

static uint8_t maybe_parse_range(const uint8_t* buffer, uint8_t len, struct range_iter* iter)
{
    uint8_t rc;
    uint8_t total_consumed = 0;
    uint8_t consumed;
    uint8_t i;
    uint16_t cellref_start;
    uint16_t cellref_end;

    rc = m_symbols_to_cellref(buffer + total_consumed, len - total_consumed, &consumed, &cellref_start);
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

    rc = m_symbols_to_cellref(buffer + total_consumed, len - total_consumed, &consumed, &cellref_end);
    if (rc != EVALUATE_OK)
        return EVALUATE_BAD_RANGE;
    total_consumed += consumed;

    return range_start(iter, cellref_start, cellref_end);
}

typedef uint8_t (*number_func)(void* state, const struct number_t* number);

static uint8_t number_iter(const uint8_t* buffer, uint8_t len, const void* state, number_func func)
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

struct max_state
{
    uint8_t first;
    struct number_t* result;
};

static uint8_t max_func(void* state, const struct number_t* number)
{
    struct max_state* local_state = state;

    if ((local_state->first) || (m_compare(number, local_state->result) > 0))
        memcpy(local_state->result, number, sizeof(struct number_t));
    local_state->first = 0;

    return EVALUATE_OK;
}

static uint8_t handle_max(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    struct max_state state;
    state.first = 1;
    state.result = result;
    return number_iter(buffer, len, &state, max_func);
}

static uint8_t min_func(void* state, const struct number_t* number)
{
    struct max_state* local_state = state;

    if ((local_state->first) || (m_compare(number, local_state->result) < 0))
        memcpy(local_state->result, number, sizeof(struct number_t));
    local_state->first = 0;

    return EVALUATE_OK;
}

static uint8_t handle_min(const uint8_t* buffer, uint8_t len, struct number_t* result)
{
    struct max_state state;
    state.first = 1;
    state.result = result;
    return number_iter(buffer, len, &state, min_func);
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
