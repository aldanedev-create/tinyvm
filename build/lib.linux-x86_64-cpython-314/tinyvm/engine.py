from . import _native
from .assembler import assemble


class VirtualMachine:
    """Python interface to the TinyVM native execution engine."""

    def __init__(self, max_stack=256, max_instructions=100_000):
        for name, value in (("max_stack", max_stack),
                            ("max_instructions", max_instructions)):
            if not isinstance(value, int) or isinstance(value, bool):
                raise TypeError(f"{name} must be an integer.")
            if value <= 0:
                raise ValueError(f"{name} must be greater than zero.")
        self.max_stack = max_stack
        self.max_instructions = max_instructions

    def execute(self, instructions):
        """Assemble and run instruction tuples; return the PRINT output."""
        bytecode = assemble(instructions)
        return _native.execute(
            bytecode,
            max_stack=self.max_stack,
            max_instructions=self.max_instructions,
        )