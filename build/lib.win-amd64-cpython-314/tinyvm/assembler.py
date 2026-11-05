import struct

OPCODES = {
    "PUSH": 0x01, "POP": 0x02, "DUP": 0x03,
    "ADD": 0x10, "SUB": 0x11, "MUL": 0x12,
    "DIV": 0x13, "MOD": 0x14,
    "EQ": 0x20, "GT": 0x21, "LT": 0x22,
    "JMP": 0x30, "JZ": 0x31,
    "PRINT": 0x40, "HALT": 0xFF,
}


def _is_integer(value):
    return isinstance(value, int) and not isinstance(value, bool)


def assemble(instructions):
    """Convert instruction tuples (with labels) into TinyVM bytecode."""
    instructions = list(instructions)
    labels = {}
    offset = 0

    # Pass 1: validate shapes, compute byte offsets, record labels.
    for item in instructions:
        if not isinstance(item, tuple) or not item:
            raise ValueError("Instructions must be non-empty tuples.")
        name = item[0]
        if not isinstance(name, str):
            raise ValueError("Instruction names must be strings.")
        name = name.upper()

        if name == "LABEL":
            if len(item) != 2 or not isinstance(item[1], str):
                raise ValueError("LABEL requires one string name.")
            if item[1] in labels:
                raise ValueError(f"Duplicate label: {item[1]}")
            labels[item[1]] = offset
        elif name == "PUSH":
            if len(item) != 2 or not _is_integer(item[1]):
                raise ValueError("PUSH requires one integer.")
            if not -(2**63) <= item[1] <= 2**63 - 1:
                raise ValueError("PUSH value must fit in signed 64 bits.")
            offset += 9
        elif name in ("JMP", "JZ"):
            if len(item) != 2:
                raise ValueError(f"{name} requires one target.")
            offset += 5
        elif name in OPCODES:
            if len(item) != 1:
                raise ValueError(f"{name} takes no operands.")
            offset += 1
        else:
            raise ValueError(f"Unknown instruction: {name}")

    # Pass 2: encode, now that label offsets are known.
    output = bytearray()
    for item in instructions:
        name = item[0].upper()
        if name == "LABEL":
            continue

        output.append(OPCODES[name])
        if name == "PUSH":
            output.extend(struct.pack("<q", item[1]))
        elif name in ("JMP", "JZ"):
            target = item[1]
            if isinstance(target, str):
                if target not in labels:
                    raise ValueError(f"Unknown label: {target}")
                target = labels[target]
            if not _is_integer(target) or not 0 <= target <= 0xFFFFFFFF:
                raise ValueError(f"{name} target must be a valid byte offset.")
            output.extend(struct.pack("<I", target))

    return bytes(output)