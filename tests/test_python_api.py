import pytest

import tinyvm


def run(program, **kwargs):
    return tinyvm.VirtualMachine(**kwargs).execute(program)


def test_add():
    assert run([("PUSH", 10), ("PUSH", 20), ("ADD",), ("PRINT",), ("HALT",)]) == [30]


def test_subtraction_order():
    assert run([("PUSH", 20), ("PUSH", 5), ("SUB",), ("PRINT",), ("HALT",)]) == [15]


def test_comparison():
    assert run([("PUSH", 10), ("PUSH", 3), ("GT",), ("PRINT",), ("HALT",)]) == [1]


def test_jump_if_zero():
    assert run([
        ("PUSH", 0),
        ("JZ", "zero_case"),
        ("PUSH", 999), ("PRINT",), ("HALT",),
        ("LABEL", "zero_case"),
        ("PUSH", 42), ("PRINT",), ("HALT",),
    ]) == [42]


def test_division_by_zero():
    with pytest.raises(ZeroDivisionError):
        run([("PUSH", 8), ("PUSH", 0), ("DIV",), ("HALT",)])


def test_stack_underflow():
    with pytest.raises(RuntimeError):
        run([("POP",), ("HALT",)])


def test_unknown_instruction():
    with pytest.raises(ValueError):
        run([("NOT_AN_OPCODE",)])


def test_unknown_label():
    with pytest.raises(ValueError):
        run([("JMP", "nowhere")])


def test_instruction_limit():
    with pytest.raises(RuntimeError):
        run([("LABEL", "again"), ("JMP", "again")], max_instructions=10)