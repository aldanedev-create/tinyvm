#include "vm.h"
#include "opcode.h"
#include "stack.h"
#include "value.h"

#include <stdio.h>
#include <stdlib.h>






static int checked_add(int64_t a, int64_t b, int64_t *out)
{
    if ((b > 0 && a > INT64_MAX - b) ||
        (b < 0 && a < INT64_MIN - b)) {
        return 0;
    }
    *out = a + b;
    return 1;
}

static int checked_sub(int64_t a, int64_t b, int64_t *out)
{
    if ((b < 0 && a > INT64_MAX + b) ||
        (b > 0 && a < INT64_MIN + b)) {
        return 0;
    }
    *out = a - b;
    return 1;
}

static int checked_mul(int64_t a, int64_t b, int64_t *out)
{
    if (a == 0 || b == 0) {
        *out = 0;
        return 1;
    }
    if ((a == -1 && b == INT64_MIN) || (b == -1 && a == INT64_MIN)) {
        return 0;
    }
    if (a > 0) {
        if ((b > 0 && a > INT64_MAX / b) ||
            (b < 0 && b < INT64_MIN / a)) {
            return 0;
        }
    } else {
        if ((b > 0 && a < INT64_MIN / b) ||
            (b < 0 && a < INT64_MAX / b)) {
            return 0;
        }
    }
    *out = a * b;
    return 1;
}


static void set_error(TinyVMError *error, TinyVMStatus code,
                      size_t offset, const char *message)
{
    if (error == NULL) {
        return;
    }
    error->code = code;
    error->byte_offset = offset;
    snprintf(error->message, sizeof(error->message), "%s", message);
}

TinyVMConfig tinyvm_default_config(void)
{
    TinyVMConfig config;
    config.max_stack = 256;
    config.max_instructions = 100000;
    return config;
}

void tinyvm_result_free(TinyVMResult *result)
{
    if (result == NULL) {
        return;
    }
    free(result->values);
    result->values = NULL;
    result->count = 0;
}

static size_t operand_size(uint8_t opcode)
{
    switch (opcode) {
        case TINY_OP_PUSH_I64:
            return TINYVM_I64_OPERAND_SIZE;
        case TINY_OP_JMP:
        case TINY_OP_JZ:
            return TINYVM_JUMP_OPERAND_SIZE;
        case TINY_OP_POP:
        case TINY_OP_DUP:
        case TINY_OP_ADD:
        case TINY_OP_SUB:
        case TINY_OP_MUL:
        case TINY_OP_DIV:
        case TINY_OP_MOD:
        case TINY_OP_EQ:
        case TINY_OP_GT:
        case TINY_OP_LT:
        case TINY_OP_PRINT:
        case TINY_OP_HALT:
            return 0;
        default:
            return (size_t)-1;
    }
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static int64_t read_i64_le(const uint8_t *p)
{
    uint64_t raw = 0;
    size_t i;

    for (i = 0; i < 8; i++) {
        raw |= ((uint64_t)p[i]) << (i * 8);
    }
    return (int64_t)raw;
}

static TinyVMStatus validate_bytecode(const uint8_t *code, size_t length,
                                      uint8_t **map_out, TinyVMError *error)
{
    size_t offset = 0;
    uint8_t *map;

    if (code == NULL || length == 0 || map_out == NULL) {
        set_error(error, TINYVM_ERROR_INVALID_ARGUMENT, 0,
                  "Bytecode must not be empty.");
        return TINYVM_ERROR_INVALID_ARGUMENT;
    }

    map = (uint8_t *)calloc(length + 1, 1);
    if (map == NULL) {
        set_error(error, TINYVM_ERROR_OUT_OF_MEMORY, 0,
                  "Could not allocate validation map.");
        return TINYVM_ERROR_OUT_OF_MEMORY;
    }

    /* Pass 1: find instruction boundaries, reject bad or truncated ones. */
    while (offset < length) {
        size_t extra;

        map[offset] = 1;
        extra = operand_size(code[offset]);

        if (extra == (size_t)-1) {
            free(map);
            set_error(error, TINYVM_ERROR_INVALID_OPCODE, offset,
                      "Unknown opcode.");
            return TINYVM_ERROR_INVALID_OPCODE;
        }
        if (extra > length - offset - 1) {
            free(map);
            set_error(error, TINYVM_ERROR_TRUNCATED_BYTECODE, offset,
                      "Instruction operand is truncated.");
            return TINYVM_ERROR_TRUNCATED_BYTECODE;
        }
        offset += 1 + extra;
    }

    /* Pass 2: every jump must land on an instruction boundary. */
    offset = 0;
    while (offset < length) {
        uint8_t op = code[offset];

        if (op == TINY_OP_JMP || op == TINY_OP_JZ) {
            uint32_t target = read_u32_le(code + offset + 1);

            if ((size_t)target >= length || map[target] == 0) {
                free(map);
                set_error(error, TINYVM_ERROR_INVALID_JUMP, offset,
                          "Jump target is not an instruction boundary.");
                return TINYVM_ERROR_INVALID_JUMP;
            }
        }
        offset += 1 + operand_size(op);
    }

    *map_out = map;
    return TINYVM_OK;
}



static int append_output(TinyVMResult *result, size_t *capacity, int64_t value)
{
    int64_t *grown;
    size_t next;

    if (result->count == *capacity) {
        next = (*capacity == 0) ? 8 : (*capacity * 2);
        if (next < *capacity || next > ((size_t)-1) / sizeof(int64_t)) {
            return 0;
        }
        grown = (int64_t *)realloc(result->values, next * sizeof(int64_t));
        if (grown == NULL) {
            return 0;
        }
        result->values = grown;
        *capacity = next;
    }

    result->values[result->count++] = value;
    return 1;
}

TinyVMStatus tinyvm_execute(
    const uint8_t *bytecode,
    size_t bytecode_length,
    const TinyVMConfig *config_input,
    TinyVMResult *result,
    TinyVMError *error
)
{
    TinyVMConfig defaults = tinyvm_default_config();
    const TinyVMConfig *config = config_input != NULL ? config_input : &defaults;
    TinyStack stack;
    uint8_t *boundaries = NULL;
    size_t ip = 0;
    size_t output_capacity = 0;
    uint64_t steps = 0;
    TinyVMStatus status;

    if (result == NULL) {
        set_error(error, TINYVM_ERROR_INVALID_ARGUMENT, 0,
                  "Result pointer is required.");
        return TINYVM_ERROR_INVALID_ARGUMENT;
    }
    result->values = NULL;
    result->count = 0;

    if (config->max_stack == 0 || config->max_instructions == 0) {
        set_error(error, TINYVM_ERROR_INVALID_ARGUMENT, 0,
                  "VM limits must be greater than zero.");
        return TINYVM_ERROR_INVALID_ARGUMENT;
    }

    status = validate_bytecode(bytecode, bytecode_length, &boundaries, error);
    if (status != TINYVM_OK) {
        return status;
    }

    if (tiny_stack_init(&stack, config->max_stack) != TINY_STACK_OK) {
        free(boundaries);
        set_error(error, TINYVM_ERROR_OUT_OF_MEMORY, 0,
                  "Could not initialize operand stack.");
        return TINYVM_ERROR_OUT_OF_MEMORY;
    }

    while (ip < bytecode_length) {
        uint8_t op;
        size_t instruction_offset = ip;
        TinyValue value;

        if (steps >= config->max_instructions) {
            status = TINYVM_ERROR_INSTRUCTION_LIMIT;
            set_error(error, status, ip, "Instruction limit exceeded.");
            goto failed;
        }
        steps++;
        op = bytecode[ip++];

        switch (op) {
            case TINY_OP_PUSH_I64:
                value = tiny_value_from_i64(read_i64_le(bytecode + ip));
                ip += TINYVM_I64_OPERAND_SIZE;
                if (tiny_stack_push(&stack, value) != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_OVERFLOW;
                    set_error(error, status, instruction_offset,
                              "Operand stack overflow.");
                    goto failed;
                }
                break;

            case TINY_OP_POP:
                if (tiny_stack_pop(&stack, &value) != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_UNDERFLOW;
                    set_error(error, status, instruction_offset,
                              "Operand stack underflow.");
                    goto failed;
                }
                break;

            case TINY_OP_DUP:
                if (tiny_stack_dup(&stack) != TINY_STACK_OK) {
                    status = stack.size >= stack.capacity
                        ? TINYVM_ERROR_STACK_OVERFLOW
                        : TINYVM_ERROR_STACK_UNDERFLOW;
                    set_error(error, status, instruction_offset,
                              "DUP failed.");
                    goto failed;
                }
                break;

            case TINY_OP_PRINT:
                if (tiny_stack_pop(&stack, &value) != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_UNDERFLOW;
                    set_error(error, status, instruction_offset,
                              "PRINT needs a value.");
                    goto failed;
                }
                if (!append_output(result, &output_capacity,
                                   tiny_value_as_i64(value))) {
                    status = TINYVM_ERROR_OUT_OF_MEMORY;
                    set_error(error, status, instruction_offset,
                              "Output allocation failed.");
                    goto failed;
                }
                break;
                
                            case TINY_OP_ADD:
            case TINY_OP_SUB:
            case TINY_OP_MUL:
            case TINY_OP_DIV:
            case TINY_OP_MOD: {
                TinyValue left, right;
                int64_t a, b, answer = 0;
                int ok = 1;

                if (tiny_stack_pop(&stack, &right) != TINY_STACK_OK ||
                    tiny_stack_pop(&stack, &left) != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_UNDERFLOW;
                    set_error(error, status, instruction_offset,
                              "Arithmetic needs two values.");
                    goto failed;
                }

                a = left.integer;
                b = right.integer;

                if (op == TINY_OP_ADD) {
                    ok = checked_add(a, b, &answer);
                } else if (op == TINY_OP_SUB) {
                    ok = checked_sub(a, b, &answer);
                } else if (op == TINY_OP_MUL) {
                    ok = checked_mul(a, b, &answer);
                } else if (op == TINY_OP_DIV) {
                    if (b == 0) {
                        status = TINYVM_ERROR_DIVISION_BY_ZERO;
                        set_error(error, status, instruction_offset,
                                  "Division by zero.");
                        goto failed;
                    }
                    if (a == INT64_MIN && b == -1) {
                        ok = 0;
                    } else {
                        answer = a / b;
                    }
                } else {
                    if (b == 0) {
                        status = TINYVM_ERROR_DIVISION_BY_ZERO;
                        set_error(error, status, instruction_offset,
                                  "Modulo by zero.");
                        goto failed;
                    }
                    answer = (a == INT64_MIN && b == -1) ? 0 : a % b;
                }

                if (!ok) {
                    status = TINYVM_ERROR_ARITHMETIC_OVERFLOW;
                    set_error(error, status, instruction_offset,
                              "Signed integer overflow.");
                    goto failed;
                }
                if (tiny_stack_push(&stack, tiny_value_from_i64(answer))
                    != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_OVERFLOW;
                    set_error(error, status, instruction_offset,
                              "Operand stack overflow.");
                    goto failed;
                }
                break;
            }

            case TINY_OP_EQ:
            case TINY_OP_GT:
            case TINY_OP_LT: {
                TinyValue left, right;
                int64_t answer;

                if (tiny_stack_pop(&stack, &right) != TINY_STACK_OK ||
                    tiny_stack_pop(&stack, &left) != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_UNDERFLOW;
                    set_error(error, status, instruction_offset,
                              "Comparison needs two values.");
                    goto failed;
                }

                if (op == TINY_OP_EQ) {
                    answer = (left.integer == right.integer);
                } else if (op == TINY_OP_GT) {
                    answer = (left.integer > right.integer);
                } else {
                    answer = (left.integer < right.integer);
                }

                if (tiny_stack_push(&stack, tiny_value_from_i64(answer))
                    != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_OVERFLOW;
                    set_error(error, status, instruction_offset,
                              "Operand stack overflow.");
                    goto failed;
                }
                break;
            }

            case TINY_OP_JMP:
                ip = (size_t)read_u32_le(bytecode + ip);
                break;

            case TINY_OP_JZ: {
                uint32_t target = read_u32_le(bytecode + ip);

                if (tiny_stack_pop(&stack, &value) != TINY_STACK_OK) {
                    status = TINYVM_ERROR_STACK_UNDERFLOW;
                    set_error(error, status, instruction_offset,
                              "JZ needs a condition value.");
                    goto failed;
                }

                if (value.integer == 0) {
                    ip = (size_t)target;
                } else {
                    ip += TINYVM_JUMP_OPERAND_SIZE;
                }
                break;
            }



            case TINY_OP_HALT:
                free(boundaries);
                tiny_stack_destroy(&stack);
                return TINYVM_OK;

            default:
                /* Arithmetic, comparisons and jumps come in later lessons. */
                status = TINYVM_ERROR_INVALID_OPCODE;
                set_error(error, status, instruction_offset,
                          "Instruction not implemented yet.");
                goto failed;
        }
    }

    free(boundaries);
    tiny_stack_destroy(&stack);
    return TINYVM_OK;

failed:
    free(boundaries);
    tiny_stack_destroy(&stack);
    tinyvm_result_free(result);
    return status;
}

