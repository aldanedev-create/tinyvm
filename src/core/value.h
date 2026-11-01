#ifndef TINYVM_VALUE_H
#define TINYVM_VALUE_H

#include <stdint.h>

typedef struct {
    int64_t integer;
} TinyValue;

TinyValue tiny_value_from_i64(int64_t value);
int64_t tiny_value_as_i64(TinyValue value);

#endif