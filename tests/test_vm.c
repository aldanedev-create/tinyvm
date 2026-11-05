#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "vm.h"
#include "opcode.h"

static void test_add_and_print(void)
{
    const uint8_t code[] = {
        TINY_OP_PUSH_I64, 10, 0, 0, 0, 0, 0, 0, 0,
        TINY_OP_PUSH_I64, 20, 0, 0, 0, 0, 0, 0, 0,
        TINY_OP_ADD,
        TINY_OP_PRINT,
        TINY_OP_HALT
    };
    TinyVMConfig config = tinyvm_default_config();
    TinyVMResult result = {0};
    TinyVMError error = {0};

    assert(tinyvm_execute(code, sizeof(code), &config, &result, &error)
           == TINYVM_OK);
    assert(result.count == 1);
    assert(result.values[0] == 30);
    tinyvm_result_free(&result);
}

static void test_division_by_zero(void)
{
    const uint8_t code[] = {
        TINY_OP_PUSH_I64, 10, 0, 0, 0, 0, 0, 0, 0,
        TINY_OP_PUSH_I64, 0, 0, 0, 0, 0, 0, 0, 0,
        TINY_OP_DIV,
        TINY_OP_HALT
    };
    TinyVMConfig config = tinyvm_default_config();
    TinyVMResult result = {0};
    TinyVMError error = {0};

    assert(tinyvm_execute(code, sizeof(code), &config, &result, &error)
           == TINYVM_ERROR_DIVISION_BY_ZERO);
    tinyvm_result_free(&result);
}

static void test_invalid_opcode(void)
{
    const uint8_t code[] = { 0x77 };
    TinyVMConfig config = tinyvm_default_config();
    TinyVMResult result = {0};
    TinyVMError error = {0};

    assert(tinyvm_execute(code, sizeof(code), &config, &result, &error)
           == TINYVM_ERROR_INVALID_OPCODE);
    tinyvm_result_free(&result);
}

int main(void)
{
    test_add_and_print();
    test_division_by_zero();
    test_invalid_opcode();
    puts("All TinyVM C tests passed.");
    return 0;
}