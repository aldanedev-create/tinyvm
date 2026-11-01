#include "stack.h"

#include <stdlib.h>

TinyStackStatus tiny_stack_init(TinyStack *stack, size_t capacity)
{
    if (stack == NULL || capacity == 0) {
        return TINY_STACK_INVALID_ARGUMENT;
    }

    stack->items = (TinyValue *)calloc(capacity, sizeof(TinyValue));
    if (stack->items == NULL) {
        stack->size = 0;
        stack->capacity = 0;
        return TINY_STACK_INVALID_ARGUMENT;
    }

    stack->size = 0;
    stack->capacity = capacity;
    return TINY_STACK_OK;
}

void tiny_stack_destroy(TinyStack *stack)
{
    if (stack == NULL) {
        return;
    }
    free(stack->items);
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

TinyStackStatus tiny_stack_push(TinyStack *stack, TinyValue value)
{
    if (stack == NULL || stack->items == NULL) {
        return TINY_STACK_INVALID_ARGUMENT;
    }
    if (stack->size >= stack->capacity) {
        return TINY_STACK_OVERFLOW;
    }
    stack->items[stack->size++] = value;
    return TINY_STACK_OK;
}

TinyStackStatus tiny_stack_pop(TinyStack *stack, TinyValue *out_value)
{
    if (stack == NULL || out_value == NULL || stack->items == NULL) {
        return TINY_STACK_INVALID_ARGUMENT;
    }
    if (stack->size == 0) {
        return TINY_STACK_UNDERFLOW;
    }
    *out_value = stack->items[--stack->size];
    return TINY_STACK_OK;
}

TinyStackStatus tiny_stack_peek(const TinyStack *stack, TinyValue *out_value)
{
    if (stack == NULL || out_value == NULL || stack->items == NULL) {
        return TINY_STACK_INVALID_ARGUMENT;
    }
    if (stack->size == 0) {
        return TINY_STACK_UNDERFLOW;
    }
    *out_value = stack->items[stack->size - 1];
    return TINY_STACK_OK;
}

TinyStackStatus tiny_stack_dup(TinyStack *stack)
{
    TinyValue value;
    TinyStackStatus status = tiny_stack_peek(stack, &value);

    if (status != TINY_STACK_OK) {
        return status;
    }
    return tiny_stack_push(stack, value);
}