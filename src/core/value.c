#include "value.h"

TinyValue tiny_value_from_i64(int64_t value)
{
    TinyValue result;
    result.integer = value;
    return result;
}

int64_t tiny_value_as_i64(TinyValue value)
{
    return value.integer;
}