#!/usr/bin/env python3

import operator
import string

operators = { '+' : operator.add, '-' : operator.sub, '*' : operator.mul, '/' : operator.ifloordiv }
operand_stack = []
operator_stack = []

# expression = "( 1 + ( ( 2 + 3 ) * ( 4 * 5 ) ) )"
expression = "1 * ((((2 + 3))))"

for this_char in expression:
    if this_char in '( ':
        pass
    elif this_char in operators.keys():
        operator_stack.append(operators[this_char])
    elif this_char in string.digits:
        operand_stack.append(int(this_char))
    elif this_char == ')':
        this_operator = operator_stack.pop()
        operand_2 = operand_stack.pop()
        operand_1 = operand_stack.pop()
        operand_stack.append(this_operator(operand_1, operand_2))

while (len(operator_stack)):
    this_operator = operator_stack.pop()
    operand_2 = operand_stack.pop()
    operand_1 = operand_stack.pop()
    operand_stack.append(this_operator(operand_1, operand_2))

print(operand_stack)
