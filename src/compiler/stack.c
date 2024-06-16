#include "compiler/stack.h"

stack_long_t* stack_long_init() {
    stack_long_t* stack = (stack_long_t*)malloc(sizeof(stack_long_t));
    stack->top = -1;
    return stack;
}

void stack_long_push(stack_long_t* stack, long value) {
    if (stack->top == 255) {
        error_throw(ERROR_COMPILER, "Stack overflow", 0);
        return;
    }

    stack->data[++(stack->top)] = value;
}

long stack_long_pop(stack_long_t* stack) {
    if (stack->top == -1) {
        error_throw(ERROR_COMPILER, "Stack underflow", 0);
        return 0;
    }

    return stack->data[(stack->top)--];
}

long stack_long_peek(stack_long_t* stack) {
    if (stack->top == -1) {
        error_throw(ERROR_COMPILER, "Stack is empty", 0);
        return 0;
    }

    return stack->data[stack->top];
}

bool stack_long_is_empty(stack_long_t* stack) {
    return stack->top == -1;
}