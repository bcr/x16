#!/usr/bin/env python3

import operator
import string

def evaluate_expression(expression):
    operators = { '+' : operator.add, '-' : operator.sub, '*' : operator.mul, '/' : operator.truediv }
    operand_stack = []
    operator_stack = []

    # expression = "( 1 + ( ( 2 + 3 ) * ( 4 * 5 ) ) )"
    # expression = "1 * ((((2 + 3))))"
    # expression = "1 + (2 * 3) + ((4 * 5) / 6)"

    for this_char in expression:
        if this_char == ' ':
            pass
        elif this_char == '(':
            operator_stack.append(this_char)
        elif this_char in operators.keys():
            operator_stack.append(operators[this_char])
        elif this_char in string.digits:
            operand_stack.append(int(this_char))
        elif this_char == ')':
            while len(operator_stack) > 0:
                this_operator = operator_stack.pop()
                if this_operator == '(':
                    break
                operand_2 = operand_stack.pop()
                operand_1 = operand_stack.pop()
                operand_stack.append(this_operator(operand_1, operand_2))

    while (len(operator_stack)):
        this_operator = operator_stack.pop()
        operand_2 = operand_stack.pop()
        operand_1 = operand_stack.pop()
        operand_stack.append(this_operator(operand_1, operand_2))

    return operand_stack.pop();

print(evaluate_expression("1 + 2 * 3 + 4 * 5 / 4"))
