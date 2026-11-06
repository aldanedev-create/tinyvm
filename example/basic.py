import tinyvm

program = [
    ("PUSH", 12),
    ("PUSH", 8),
    ("MUL",),
    ("PRINT",),
    ("HALT",),
]

print("Output:", tinyvm.VirtualMachine().execute(program))