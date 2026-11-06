# Quick Start

Get started with TinyVM and execute your first stack-based bytecode program.

## 1. Installation

Install TinyVM:

```bash
python -m pip install tinyvm-Flaxot
```

Verify the installation:

```bash
python -c "import tinyvm; print(tinyvm.__version__)"
```

## 2. Create your first program

Create a file named `hello.py`:

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

instructions = [
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
]

result = vm.execute(instructions)

print(result)
```

Run it:

```bash
python hello.py
```

Output:

```text
[30]
```

## 3. Understand the instructions

TinyVM executes instructions using an operand stack.

Consider this program:

```python
instructions = [
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
]
```

The execution works as follows:

| Step | Instruction | Stack |
|---|---|---|
| 1 | `PUSH 10` | `[10]` |
| 2 | `PUSH 20` | `[10, 20]` |
| 3 | `ADD` | `[30]` |
| 4 | `PRINT` | `[]` |
| 5 | `HALT` | `[]` |

`ADD` removes the two top values, adds them, and pushes the result.

`PRINT` removes the top value and stores it in the VM output.

## 4. Use more arithmetic operations

TinyVM supports addition, subtraction, multiplication, division, and modulo.

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

result = vm.execute([
    ("PUSH", 100),
    ("PUSH", 25),
    ("SUB",),
    ("PRINT",),

    ("PUSH", 10),
    ("PUSH", 5),
    ("MUL",),
    ("PRINT",),

    ("HALT",),
])

print(result)
```

Output:

```text
[75, 50]
```

Arithmetic instructions consume two values. The first value pushed is the left operand, and the second value pushed is the right operand.

For example:

```python
("PUSH", 20),
("PUSH", 5),
("SUB",),
```

Produces `15`.

## 5. Compare values

Comparison instructions return integer values:

- `1` means true.
- `0` means false.

Example:

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

result = vm.execute([
    ("PUSH", 10),
    ("PUSH", 10),
    ("EQ",),
    ("PRINT",),

    ("PUSH", 20),
    ("PUSH", 10),
    ("GT",),
    ("PRINT",),

    ("PUSH", 5),
    ("PUSH", 10),
    ("LT",),
    ("PRINT",),

    ("HALT",),
])

print(result)
```

Output:

```text
[1, 1, 1]
```

## 6. Use labels and jumps

TinyVM supports named labels, unconditional jumps, and conditional jumps.

Labels are declared using:

```python
("LABEL", "label_name")
```

For example, this program uses a conditional jump:

```python
from tinyvm import VirtualMachine

vm = VirtualMachine()

result = vm.execute([
    ("PUSH", 0),
    ("JZ", "zero"),
    ("PUSH", 100),
    ("PRINT",),
    ("HALT",),

    ("LABEL", "zero"),
    ("PUSH", 42),
    ("PRINT",),
    ("HALT",),
])

print(result)
```

Output:

```text
[42]
```

`JZ` removes the top stack value. If that value is zero, execution jumps to the specified label. Otherwise, execution continues with the next instruction.

`JMP` always jumps to its target.

## 7. Configure execution limits

You can configure the maximum stack capacity and instruction count.

```python
from tinyvm import VirtualMachine

vm = VirtualMachine(
    max_stack=512,
    max_instructions=200_000,
)
```

| Parameter | Default | Description |
|---|---:|---|
| `max_stack` | `256` | Maximum number of stack values |
| `max_instructions` | `100000` | Maximum number of executed instructions |

Both values must be positive integers.

## 8. Assemble bytecode directly

You can use TinyVM's assembler without creating a `VirtualMachine`.

```python
from tinyvm import assemble

bytecode = assemble([
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
])

print(bytecode)
```

The assembler returns a `bytes` object containing the encoded instructions.

You can pass this bytecode to the native execution function:

```python
from tinyvm import _native, assemble

bytecode = assemble([
    ("PUSH", 10),
    ("PUSH", 20),
    ("ADD",),
    ("PRINT",),
    ("HALT",),
])

result = _native.execute(bytecode)

print(result)
```

The native function also accepts execution limits:

```python
result = _native.execute(
    bytecode,
    max_stack=512,
    max_instructions=200_000,
)
```

The normal application interface is `VirtualMachine`. Direct use of `_native` is intended for users who need lower-level access.

## 9. Next steps

- Read the [API Reference](api.md) for all public methods and instruction formats.
- Explore the [TinyVM repository](https://github.com/aldanedev-create/tinyvm).
- Experiment with arithmetic, comparisons, labels, and jumps.

You now have everything needed to write and execute basic TinyVM programs.