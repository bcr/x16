#include "cell.h"
#include "expr.h"
#include "util.h"
#include "expr_iter.h"

uint8_t range_start(struct range_iter *iter, uint16_t start, uint16_t end)
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

void range_next(struct range_iter *iter)
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

void comma_next(struct comma_iter* iter)
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

void comma_start(struct comma_iter* iter, const uint8_t* buffer, uint8_t len)
{
    iter->buffer = buffer;
    iter->buffer_len = len;
    iter->first = 1;
    iter->pos = 0;
    iter->len = 0;
    iter->last = 0;
}

uint8_t maybe_parse_range(const uint8_t* buffer, uint8_t len, struct range_iter* iter)
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

uint8_t number_iter(const uint8_t* buffer, uint8_t len, void* state, number_func func)
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
