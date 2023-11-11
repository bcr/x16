#include <string.h>

#include "dc_math.h"

#include "expr.h"

#define MAX_OPERATORS 10
#define MAX_OPERANDS 10

uint8_t e_evaluate(const uint8_t* expression, uint8_t len, struct number_t* result)
{
    uint8_t operators[MAX_OPERATORS];
    int8_t current_operator = 0;
    struct number_t operands[MAX_OPERANDS];
    int8_t current_operand = 0;
    uint8_t index = 0;
    uint8_t this_symbol;
    uint8_t consumed;
    void (*operator)(const struct number_t* a, const struct number_t* b, struct number_t* result) = NULL;

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
                switch (operators[current_operator - 1])
                {
                    case '/':
                        operator = m_divide;
                        break;
                    case '*':
                        operator = m_multiply;
                        break;
                    case '+':
                        operator = m_add;
                        break;
                    case '-':
                        operator = m_subtract;
                        break;
                    case '(':
                        break;
                    default:
                        return EVALUATE_UNKNOWN_OPERATOR;
                }
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
            m_symbols_to_number(expression + index, len - index, &(operands[current_operand]), &consumed);
            ++current_operand;
        }
        index += consumed;
    }

    while (current_operator > 0)
    {
        switch (operators[current_operator - 1])
        {
            case '/':
                operator = m_divide;
                break;
            case '*':
                operator = m_multiply;
                break;
            case '+':
                operator = m_add;
                break;
            case '-':
                operator = m_subtract;
                break;
            case '(':
                break;
            default:
                return EVALUATE_UNKNOWN_OPERATOR;
        }
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