# TinyVM

**A small C-powered stack-based bytecode virtual machine for Python.**

TinyVM is a lightweight virtual machine written in C with a Python interface. It provides a simple stack-based instruction set, a Python assembler, and a native execution engine.

Write instructions in Python, assemble them into bytecode, and execute them using TinyVM's native engine.

## Features

- **Native execution:** VM execution is implemented in C.
- **Python interface:** Write and execute programs using ordinary Python.
- **Stack-based architecture:** Operate on signed 64-bit integer values.
- **Built-in assembler:** Convert Python instruction tuples into bytecode.
- **Labels and jumps:** Build programs with named labels and conditional or unconditional jumps.
- **Execution limits:** Configure maximum stack capacity and instruction count.
- **Error handling:** Detect invalid bytecode, stack errors, division by zero, arithmetic overflow, and invalid jumps.
- **Cross-platform wheels:** Prebuilt wheels are provided for supported CPython platforms.

## Installation

Install TinyVM using pip:

```bash
python -m pip install tinyvm-Flaxot
```

Import it using the Python package name:

```python
import tinyvm
```

## Quick example

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

result = vm.execute([
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
])

print(result)
```

Output:

```text
[30]
```

TinyVM executes the instructions and returns the values collected by `PRINT`.

## How it works

TinyVM uses a stack-based execution model.

1. `PUSH` places a value on the stack.
2. Arithmetic instructions consume operands and push their results.
3. Comparison instructions push `1` or `0`.
4. Jump instructions change the instruction pointer.
5. `PRINT` removes a value from the stack and adds it to the output.
6. `HALT` stops execution.

Programs can be written as Python instruction tuples or assembled into bytecode directly.

## Instruction set

| Instruction | Description |
|---|---|
| `PUSH` | Push a signed 64-bit integer |
| `POP` | Remove the top stack value |
| `DUP` | Duplicate the top stack value |
| `ADD` | Add two values |
| `SUB` | Subtract two values |
| `MUL` | Multiply two values |
| `DIV` | Divide two values |
| `MOD` | Calculate the remainder |
| `EQ` | Compare two values for equality |
| `GT` | Check whether the first value is greater |
| `LT` | Check whether the first value is smaller |
| `JMP` | Jump to a bytecode offset or label |
| `JZ` | Jump when the top value is zero |
| `PRINT` | Collect the top value as output |
| `HALT` | Stop execution |

See [API Reference](docs/api.md) for instruction behavior, arguments, and exceptions.

## Configuration

You can configure the VM's stack capacity and maximum instruction count:

```python
from tinyvm import VirtualMachine

vm = VirtualMachine(
    max_stack=512,
    max_instructions=200_000,
)
```

Defaults:

| Setting | Default |
|---|---:|
| `max_stack` | `256` |
| `max_instructions` | `100000` |

Both limits must be positive integers.

## Documentation

- [Quick Start](docs/quick-start.md)
- [API Reference](docs/api.md)

## Requirements

- Python 3.9 or newer
- CPython for the native extension wheels

## Development

Clone the repository:

```bash
git clone https://github.com/aldanedev-create/tinyvm.git
cd tinyvm
```

Install the build and testing dependencies:

```bash
python -m pip install --upgrade pip setuptools wheel build pytest
```

Build the package:

```bash
python -m build
```

Run the tests:

```bash
python -m pytest
```

## License

TinyVM is distributed under the MIT License. See [LICENSE](LICENSE).