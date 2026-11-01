#include <stdio.h>

#include "stack.h"
#include "value.h"

int main(void)
{
    TinyStack stack;
    TinyValue value;

    if (tiny_stack_init(&stack, 8) != TINY_STACK_OK) {
        fprintf(stderr, "Could not initialize stack.\n");
        return 1;
    }

    tiny_stack_push(&stack, tiny_value_from_i64(10));
    tiny_stack_push(&stack, tiny_value_from_i64(20));
    tiny_stack_dup(&stack);

    while (tiny_stack_pop(&stack, &value) == TINY_STACK_OK) {
        printf("%lld\n", (long long)tiny_value_as_i64(value));
    }

    tiny_stack_destroy(&stack);
    return 0;
}