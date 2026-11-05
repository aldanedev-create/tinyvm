#ifndef TINYVM_VM_H
#define TINYVM_VM_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TINYVM_OK = 0,
    TINYVM_ERROR_INVALID_ARGUMENT,
    TINYVM_ERROR_INVALID_OPCODE,
    TINYVM_ERROR_TRUNCATED_BYTECODE,
    TINYVM_ERROR_INVALID_JUMP,
    TINYVM_ERROR_STACK_UNDERFLOW,
    TINYVM_ERROR_STACK_OVERFLOW,
    TINYVM_ERROR_DIVISION_BY_ZERO,
    TINYVM_ERROR_ARITHMETIC_OVERFLOW,
    TINYVM_ERROR_INSTRUCTION_LIMIT,
    TINYVM_ERROR_OUT_OF_MEMORY
} TinyVMStatus;

typedef struct {
    TinyVMStatus code;
    size_t byte_offset;
    char message[128];
} TinyVMError;

typedef struct {
    int64_t *values;
    size_t count;
} TinyVMResult;

typedef struct {
    size_t max_stack;
    uint64_t max_instructions;
} TinyVMConfig;

TinyVMConfig tinyvm_default_config(void);

TinyVMStatus tinyvm_execute(
    const uint8_t *bytecode,
    size_t bytecode_length,
    const TinyVMConfig *config,
    TinyVMResult *result,
    TinyVMError *error
);

void tinyvm_result_free(TinyVMResult *result);

#endif