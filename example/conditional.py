import tinyvm

program = [
    ("PUSH", 10),
    ("PUSH", 10),
    ("EQ",),
    ("JZ", "not_equal"),
    ("PUSH", 1),
    ("PRINT",),
    ("HALT",),
    ("LABEL", "not_equal"),
    ("PUSH", 0),
    ("PRINT",),
    ("HALT",),
]

print("TinyVM-Results:", tinyvm.VirtualMachine().execute(program))