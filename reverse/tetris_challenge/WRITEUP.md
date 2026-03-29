# Tetris Challenge - AIS3 2026 Pre-Exam Writeup

## Challenge Overview

**Category:** Reverse Engineering  
**Difficulty:** Medium-Hard  
**Flag:** `AIS3{T3tr1s_P4tt3rn_M4st3r!}`

## Description

A Tetris game binary that hides a secret flag. Players must discover the correct pattern to arrange blocks in order to reveal the flag.

## Initial Analysis

### File Information
```bash
$ file tetris
tetris: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, stripped
```

The binary is:
- 64-bit ELF executable
- **Statically linked** - all libraries embedded (makes it ~1.9MB)
- **Stripped** - no debug symbols
- Has renamed/obfuscated sections

### Anti-Analysis Techniques

The binary employs multiple protection mechanisms:

1. **Anti-Disassembler Tricks:**
   - Fake CALL instructions (`0xe8` opcode in dead code)
   - Overlapping instructions
   - Fake RIP-relative addressing
   - Jump-to-middle-of-instruction patterns

2. **Anti-Debug Checks:**
   - `ptrace(PTRACE_TRACEME)` - prevents debugger attachment
   - `/proc/self/status` TracerPid check
   - Timing checks - detects if execution is too slow
   - SIGTRAP handler

3. **Control Flow Flattening:**
   - 256-entry jump table
   - State machine-based execution
   - Many junk/trap states

## Reverse Engineering Process

### Step 1: Bypass Anti-Debug

When analyzing with a debugger, the binary detects debugging and corrupts the flag decryption. We can either:
- Patch out anti-debug checks
- Use static analysis only
- Emulate with tools like Unicorn

### Step 2: Identify the Encryption

Looking for crypto-related code, we find:

1. **Encrypted flag data** at a fixed location:
```c
unsigned char encFlag[] = {
    0x2e, 0xa5, 0x56, 0x46, 0x0d, 0x7c, 0x8e, 0xdc,
    0x83, 0x6f, 0x30, 0x83, 0xff, 0xf8, 0xa5, 0x5c,
    0xd0, 0x76, 0xd8, 0xcd, 0x99, 0xdc, 0x3f, 0x39,
    0x9d, 0x65, 0x70, 0x64, 0x0
};
```

2. **RC4-like decryption function** (recognizable by the 256-byte S-box, KSA, and PRGA)

3. **Key derivation** using FNV-1a hash variant

### Step 3: Find the Winning Pattern

The key insight is that the decryption key is derived from a specific "winning pattern" - a 4x10 grid of colored blocks that must be created in the game.

By analyzing the pattern comparison in `state_pattern_compare`, we find:
```c
const int WINNING_PATTERN[4][BOARD_WIDTH] = {
    {5, 0, 5, 0, 1, 0, 4, 4, 4, 0},
    {5, 5, 5, 0, 1, 0, 4, 0, 0, 0},
    {5, 0, 5, 0, 1, 0, 0, 4, 4, 0},
    {5, 0, 5, 0, 1, 0, 4, 4, 0, 3}
};
```

Where colors are: 1=Cyan, 2=Yellow, 3=Magenta, 4=Green, 5=Red, 6=Blue, 7=White

This pattern visually represents "AIS3" spelled with Tetris blocks!

### Step 4: Key Derivation Algorithm

The key is derived using:
1. FNV-1a hash initialization: `0x811c9dc5`
2. For each cell in pattern: `hash = (hash ^ cell) * 0x01000193`
3. Linear Congruential Generator to expand to 24 bytes

```python
def derive_key_from_pattern(pattern):
    hash_val = 0x811c9dc5
    for i in range(4):
        for j in range(10):
            hash_val ^= pattern[i][j]
            hash_val = (hash_val * 0x01000193) & 0xFFFFFFFF
    
    key = []
    for i in range(24):
        key.append((hash_val >> ((i % 4) * 8)) & 0xFF)
        hash_val = (hash_val * 1103515245 + 12345) & 0x7FFFFFFF
    return bytes(key)
```

### Step 5: RC4 Decryption

Standard RC4 implementation - since RC4 is symmetric, encryption = decryption:

```python
def rc4_decrypt(data, key):
    S = list(range(256))
    j = 0
    for i in range(256):
        j = (j + S[i] + key[i % len(key)]) % 256
        S[i], S[j] = S[j], S[i]
    
    i = j = 0
    result = []
    for byte in data:
        i = (i + 1) % 256
        j = (j + S[i]) % 256
        S[i], S[j] = S[j], S[i]
        result.append(byte ^ S[(S[i] + S[j]) % 256])
    return bytes(result)
```

## Solution

Running our solve script:

```bash
$ python3 solve.py
============================================================
  Tetris Challenge Solver - AIS3 2026 Pre-Exam
============================================================

[*] Winning Pattern Visualization:
╔════════════════════╗
║██  ██  ██  ██████  ║
║██████  ██  ██      ║
║██  ██  ██    ████  ║
║██  ██  ██  ████  ██║
╚════════════════════╝

[*] Deriving encryption key from pattern...
[+] Derived key (24 bytes):
    6331e236bf039b33dbccc331b738250653b2e011af672927

[*] Decrypting flag...

============================================================
[+] FLAG: AIS3{T3tr1s_P4tt3rn_M4st3r!}
============================================================
```

## Alternative Solution: Playing the Game

If you manage to arrange the Tetris blocks to match the winning pattern (the first 4 rows should spell "AIS3" using colored blocks), pressing 'P' in the game will verify the pattern and display the flag.

However, this is nearly impossible due to:
1. Random piece generation
2. The specific color requirements
3. Anti-debug checks that interfere

## Key Takeaways

1. **Static analysis** is essential when anti-debug is present
2. **Pattern recognition**: Look for crypto primitives (FNV, RC4 S-box patterns)
3. **Control flow flattening** can be reversed by tracing state transitions
4. **Anti-disassembler tricks** often have dead code that can be identified
5. **The flag format hint**: The pattern spells "AIS3" - a clever design!

## Tools Used

- Ghidra / IDA Pro - for disassembly and decompilation
- Python - for key derivation and decryption
- Binary Ninja / radare2 - alternative analysis

## Files

- `tetris` - Challenge binary (statically linked, ~1.9MB)
- `solve.py` - Solution script
- `README.md` - Challenge description for players

---

**Flag:** `AIS3{T3tr1s_P4tt3rn_M4st3r!}`
