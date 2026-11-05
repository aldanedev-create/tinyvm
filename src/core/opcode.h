#ifndef TINYVM_OPCODE_H
#define TINYVM_OPCODE_H

#include <stdint.h>

typedef enum {
    TINY_OP_PUSH_I64 = 0x01,
    TINY_OP_POP      = 0x02,
    TINY_OP_DUP      = 0x03,
    TINY_OP_ADD      = 0x10,
    TINY_OP_SUB      = 0x11,
    TINY_OP_MUL      = 0x12,
    TINY_OP_DIV      = 0x13,
    TINY_OP_MOD      = 0x14,
    TINY_OP_EQ       = 0x20,
    TINY_OP_GT       = 0x21,
    TINY_OP_LT       = 0x22,
    TINY_OP_JMP      = 0x30,
    TINY_OP_JZ       = 0x31,
    TINY_OP_PRINT    = 0x40,
    TINY_OP_HALT     = 0xFF
} TinyOpcode;

#define TINYVM_I64_OPERAND_SIZE 8u
#define TINYVM_JUMP_OPERAND_SIZE 4u

#endif