# API Reference

This document describes TinyVM's Python interface, assembler, execution configuration, instruction set, and errors.

## Contents

- [Package](#package)
- [VirtualMachine](#virtualmachine)
- [assemble](#assemble)
- [Instruction Reference](#instruction-reference)
- [Bytecode Format](#bytecode-format)
- [Native Execution](#native-execution)
- [Exceptions](#exceptions)

## Package

Install:

```bash
python -m pip install tinyvm-Flaxot
```

Import:

```python
import tinyvm
```

Public exports:

```python
from tinyvm import VirtualMachine, assemble
```

Version:

```python
print(tinyvm.__version__)
```

## VirtualMachine

The `VirtualMachine` class provides the primary Python interface for executing TinyVM programs.

### Constructor

```python
VirtualMachine(
    max_stack=256,
    max_instructions=100_000,
)
```

| Parameter | Type | Default | Description |
|---|---|---:|---|
| `max_stack` | `int` | `256` | Maximum operand stack capacity |
| `max_instructions` | `int` | `100000` | Maximum number of executed instructions |

Both parameters must be positive integers. Boolean values are not accepted as integers.

Example:

```python
from tinyvm import VirtualMachine

vm = VirtualMachine(
    max_stack=512,
    max_instructions=200_000,
)
```

### execute()

Execute a sequence of instruction tuples.

```python
vm.execute(instructions)
```

| Parameter | Type | Description |
|---|---|---|
| `instructions` | Iterable of tuples | Instructions to assemble and execute |

Returns:

```python
list[int]
```

The returned list contains values collected by `PRINT`, in execution order.

Example:

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

output = vm.execute([
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
])

assert output == [30]
```

The VM stops when it executes `HALT` or reaches the end of the bytecode.

## assemble()

The `assemble()` function converts instruction tuples into encoded bytecode.

Import:

```python
from tinyvm import assemble
```

Signature:

```python
assemble(instructions)
```

Parameter:

| Parameter | Type | Description |
|---|---|---|
| `instructions` | Iterable of tuples | Instruction definitions and optional labels |

Returns:

```python
bytes
```

Example:

```python
from tinyvm import assemble

bytecode = assemble([
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
])

assert isinstance(bytecode, bytes)
```

### Instruction tuple format

Instructions are represented as tuples.

Instructions without operands:

```python
("ADD",)
("SUB",)
("MUL",)
("PRINT",)
("HALT",)
```

Instructions with operands:

```python
("PUSH", 100)
("JMP", "start")
("JZ", "end")
```

Instruction names are case-insensitive.

### Labels

Declare a label using `LABEL`:

```python
("LABEL", "start")
```

Labels do not generate bytecode. They identify the byte offset of an instruction.

Example:

```python
bytecode = assemble([
    ("PUSH", 0),
    ("JZ", "zero"),
    ("HALT",),

    ("LABEL", "zero"),
    ("PUSH", 42),
    ("PRINT",),
    ("HALT",),
])
```

Jump targets may also be specified as numeric byte offsets.

## Instruction Reference

TinyVM uses a stack-based instruction set.

### Stack instructions

| Instruction | Operand | Behavior |
|---|---|---|
| `PUSH` | Signed 64-bit integer | Pushes a value onto the stack |
| `POP` | None | Removes the top value |
| `DUP` | None | Duplicates the top value |

Examples:

```python
("PUSH", 10)
("DUP",)
("POP",)
```

`DUP` leaves the original value on the stack and pushes a copy.

### Arithmetic instructions

| Instruction | Behavior |
|---|---|
| `ADD` | Adds the two top values |
| `SUB` | Subtracts the top value from the value beneath it |
| `MUL` | Multiplies the two top values |
| `DIV` | Divides the value beneath the top value by the top value |
| `MOD` | Calculates the remainder of the same division |

Arithmetic instructions consume two values and push one result.

For example:

```python
("PUSH", 20),
("PUSH", 5),
("SUB",),
```

The resulting stack contains:

```text
[15]
```

Division and modulo by zero raise `ZeroDivisionError`.

Arithmetic uses signed 64-bit integers. Operations that overflow raise `OverflowError`.

### Comparison instructions

| Instruction | Behavior |
|---|---|
| `EQ` | Pushes `1` if the values are equal, otherwise `0` |
| `GT` | Pushes `1` if the lower value is greater than the top value |
| `LT` | Pushes `1` if the lower value is less than the top value |

Each comparison consumes two values and pushes one integer result.

Example:

```python
("PUSH", 10),
("PUSH", 5),
("GT",),
```

The resulting stack contains:

```text
[1]
```

### Control-flow instructions

| Instruction | Operand | Behavior |
|---|---|---|
| `JMP` | Label or byte offset | Always jumps to the target |
| `JZ` | Label or byte offset | Jumps when the popped value is zero |
| `HALT` | None | Stops execution |

`JZ` consumes the condition value from the stack.

Example:

```python
[
    ("PUSH", 0),
    ("JZ", "zero"),
    ("HALT",),

    ("LABEL", "zero"),
    ("PUSH", 42),
    ("PRINT",),
    ("HALT",),
]
```

### Output instruction

| Instruction | Behavior |
|---|---|
| `PRINT` | Pops the top stack value and appends it to the execution result |

Example:

```python
[
    ("PUSH", 42),
    ("PRINT",),
    ("HALT",),
]
```

Returns:

```python
[42]
```

`PRINT` does not print directly to the terminal. It collects values for the Python result list.

## Bytecode Format

TinyVM uses a compact instruction encoding.

### Opcode table

| Instruction | Opcode |
|---|---:|
| `PUSH` | `0x01` |
| `POP` | `0x02` |
| `DUP` | `0x03` |
| `ADD` | `0x10` |
| `SUB` | `0x11` |
| `MUL` | `0x12` |
| `DIV` | `0x13` |
| `MOD` | `0x14` |
| `EQ` | `0x20` |
| `GT` | `0x21` |
| `LT` | `0x22` |
| `JMP` | `0x30` |
| `JZ` | `0x31` |
| `PRINT` | `0x40` |
| `HALT` | `0xFF` |

### Operand encoding

| Instruction | Operand size | Encoding |
|---|---:|---|
| `PUSH` | 8 bytes | Signed 64-bit little-endian integer |
| `JMP` | 4 bytes | Unsigned 32-bit little-endian byte offset |
| `JZ` | 4 bytes | Unsigned 32-bit little-endian byte offset |
| Other instructions | 0 bytes | No operand |

Jump offsets refer to byte positions in the encoded program, not instruction indexes.

The assembler calculates label offsets automatically.

## Native Execution

TinyVM's native extension is available as:

```python
from tinyvm import _native
```

The native execution function accepts encoded bytecode directly.

### execute()

```python
_native.execute(
    bytecode,
    max_stack=256,
    max_instructions=100_000,
)
```

| Parameter | Type | Default | Description |
|---|---|---:|---|
| `bytecode` | `bytes` | Required | Encoded TinyVM instructions |
| `max_stack` | `int` | `256` | Maximum stack capacity |
| `max_instructions` | `int` | `100000` | Maximum execution steps |

Returns:

```python
list[int]
```

Example:

```python
from tinyvm import _native, assemble

bytecode = assemble([
    ("PUSH", 15),
    ("PUSH", 5),
    ("DIV",),
    ("PRINT",),
    ("HALT",),
])

result = _native.execute(bytecode)

print(result)
```

Output:

```text
[3]
```

The `VirtualMachine` class uses this native execution function internally.

## Exceptions

TinyVM reports errors through Python exceptions.

| Exception | Condition |
|---|---|
| `TypeError` | Invalid Python argument type |
| `ValueError` | Invalid instruction definitions, invalid opcode, truncated bytecode, invalid jump, or invalid execution limits |
| `RuntimeError` | Stack underflow, stack overflow, or instruction limit exceeded |
| `ZeroDivisionError` | Division or modulo by zero |
| `OverflowError` | Signed 64-bit arithmetic overflow |
| `MemoryError` | Native memory allocation failure |

### Example: division by zero

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

try:
    vm.execute([
        ("PUSH", 10),
        ("PUSH", 0),
        ("DIV",),
        ("PRINT",),
        ("HALT",),
    ])
except ZeroDivisionError as error:
    print(f"VM error: {error}")
```

### Example: invalid instruction

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

try:
    vm.execute([
        ("UNKNOWN",),
    ])
except ValueError as error:
    print(f"Invalid program: {error}")
```

Native execution errors include a byte offset identifying the location associated with the error.

## See Also

- [Quick Start](quick-start.md)
- [Project README](../README.md)
- [GitHub Repository](https://github.com/aldanedev-create/tinyvm)