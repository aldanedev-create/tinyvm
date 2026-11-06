"""TinyVM: a small bytecode virtual machine."""

from .assembler import assemble
from .engine import VirtualMachine

__all__ = ["VirtualMachine", "assemble"]
__version__ = "0.1.0"