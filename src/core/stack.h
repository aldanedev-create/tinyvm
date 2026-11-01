#ifndef TINYVM_STACK_H
#define TINYVM_STACK_H

#include <stddef.h>
#include "value.h"

typedef enum {
    TINY_STACK_OK = 0,
    TINY_STACK_UNDERFLOW,
    TINY_STACK_OVERFLOW,
    TINY_STACK_INVALID_ARGUMENT
} TinyStackStatus;

typedef struct {
    TinyValue *items;
    size_t size;
    size_t capacity;
} TinyStack;

TinyStackStatus tiny_stack_init(TinyStack *stack, size_t capacity);
void tiny_stack_destroy(TinyStack *stack);
TinyStackStatus tiny_stack_push(TinyStack *stack, TinyValue value);
TinyStackStatus tiny_stack_pop(TinyStack *stack, TinyValue *out_value);
TinyStackStatus tiny_stack_peek(const TinyStack *stack, TinyValue *out_value);
TinyStackStatus tiny_stack_dup(TinyStack *stack);

#endif