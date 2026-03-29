# Tetris Challenge - Reverse Engineering

## Description

A classic game of Tetris with a twist. Create the right pattern to reveal the flag!

## Challenge Info

- **Category**: Reverse Engineering
- **Difficulty**: Hard
- **Points**: 400
- **Flag Format**: `AIS3{...}`

## Files

- `tetris` - The challenge binary

## Technical Details

The binary features:
- 64-bit statically linked ELF executable (~1.9MB)
- Various anti-reversing protections
- Control flow flattening with 256-entry jump table
- Anti-disassembler techniques

## Hints

1. The game is more than just clearing lines...
2. What happens when you press 'P'?
3. Colors matter. Patterns matter.
4. Static analysis might be easier than dynamic analysis
5. Look for the FNV hash and RC4 encryption

## Solution (ADMIN ONLY)

<details>
<summary>Click to reveal solution</summary>

### Overview
The binary is a Tetris game with multiple obfuscation techniques:
1. **Control Flow Flattening**: 256-entry jump table state machine
2. **Anti-Debugging**: ptrace, /proc/self/status TracerPid, timing checks
3. **Anti-Disassembler**: Fake instructions, overlapping code, junk states
4. **Junk Code**: Fake functions and variables that never execute meaningfully
5. **Static Linking**: Makes the binary larger and harder to analyze

### The Pattern
Players need to create a specific pattern in the first 4 rows of the board:

```
Row 0: [R, _, R, _, C, _, G, G, G, _]
Row 1: [R, R, R, _, C, _, G, _, _, _]
Row 2: [R, _, R, _, C, _, _, G, G, _]
Row 3: [R, _, R, _, C, _, G, G, _, M]
```

Where:
- R = Red (5)
- C = Cyan (1)
- G = Green (4)
- M = Magenta (3)
- _ = Empty (0)

This pattern visually spells "AIS3"!

### Key Extraction
When the pattern matches, a key is derived using FNV-1a hash of the pattern, then the encrypted flag is decrypted using RC4.

### Flag
`AIS3{T3tr1s_P4tt3rn_M4st3r!}`

### Bypass Techniques
1. **Static analysis**: Extract the WINNING_PATTERN array and encrypted flag, implement the key derivation
2. **Patch anti-debug**: NOP out the ptrace call and TracerPid check
3. **Dynamic analysis**: Use LD_PRELOAD to hook ptrace or use frida/qiling

### Solution Script
See `solve.py` for the full solution.

</details>

## Build Instructions (ADMIN)

```bash
chmod +x build.sh
./build.sh
```

Note: Requires static library support (`apt install libc6-dev`)
