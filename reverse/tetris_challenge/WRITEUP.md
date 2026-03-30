# Tetris Challenge - WRITEUP

**Category:** Reverse Engineering  
**Difficulty:** Medium  
**Flag:** `AIS3{T3tr1s_P4tt3rn_M4st3r!}`

## Challenge Description

A Tetris game binary with heavy obfuscation. Players must find the secret pattern to reveal the flag.

## Obfuscation Techniques Used

1. **100,000 Junk Functions** - Massive number of dummy functions to confuse disassemblers
2. **100,000-Entry Jump Table** - Control flow flattening with huge indirect jump table
3. **Rogue Bytes Macros** - Anti-disassembly techniques (disabled in final build)
4. **Junk Code Blocks** - Meaningless code interleaved with real logic

## Solution

### Step 1: Initial Analysis

Load the binary in a disassembler (IDA/Ghidra/Binary Ninja). You'll notice:
- Huge binary size due to junk functions
- A large jump table array
- Encrypted data that looks like a flag

### Step 2: Find the Encrypted Flag

Search for byte sequences or string references. The encrypted flag is stored as:

```c
unsigned char encFlag[] = {
    0x2e, 0xa5, 0x56, 0x46, 0x0d, 0x7c, 0x8e, 0xdc,
    0x83, 0x6f, 0x30, 0x83, 0xff, 0xf8, 0xa5, 0x5c,
    0xd0, 0x76, 0xd8, 0xcd, 0x99, 0xdc, 0x3f, 0x39,
    0x9d, 0x65, 0x70, 0x64
};
```

### Step 3: Identify the Winning Pattern

Look for a 4x10 integer array that represents the winning Tetris pattern:

```c
const int WINNING_PATTERN[4][BOARD_WIDTH] = {
    {5, 0, 5, 0, 1, 0, 4, 4, 4, 0},
    {5, 5, 5, 0, 1, 0, 4, 0, 0, 0},
    {5, 0, 5, 0, 1, 0, 0, 4, 4, 0},
    {5, 0, 5, 0, 1, 0, 4, 4, 0, 3}
};
```

Visual representation (where numbers represent Tetromino types):
```
M   I  LLL
MMM I  L
M   I   LL
M   I  LL T
```

### Step 4: Understand the Decryption

The flag decryption uses two algorithms:

1. **Key Derivation (FNV-1a + LCG):**
   ```c
   // FNV-1a hash of pattern
   unsigned int hash = 0x811c9dc5;
   for each cell in pattern:
       hash ^= cell
       hash *= 0x01000193
   
   // Generate 24-byte key using LCG
   for i in 0..23:
       key[i] = (hash >> ((i % 4) * 8)) & 0xFF
       hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF
   ```

2. **RC4 Decryption:**
   - Standard RC4 algorithm
   - Key length: 24 bytes
   - Symmetric (encrypt = decrypt)

### Step 5: Decrypt the Flag

Run the solver script:

```bash
python3 solve.py
```

Or implement manually:

```python
# FNV-1a hash of pattern
hash_val = 0x811c9dc5
for row in WINNING_PATTERN:
    for cell in row:
        hash_val ^= cell
        hash_val = (hash_val * 0x01000193) & 0xFFFFFFFF

# Generate key
key = bytearray(24)
for i in range(24):
    key[i] = (hash_val >> ((i % 4) * 8)) & 0xFF
    hash_val = (hash_val * 1103515245 + 12345) & 0x7FFFFFFF

# RC4 decrypt
flag = rc4_decrypt(ENC_FLAG, key)
print(flag)  # AIS3{T3tr1s_P4tt3rn_M4st3r!}
```

## Alternative Solution: Play the Game

You can also solve this by actually playing Tetris and creating the winning pattern on the bottom 4 rows of the board! Press 'P' to check your pattern.

## Key Takeaways

1. Don't be intimidated by large binaries - focus on finding the important parts
2. Look for constants (magic numbers like FNV-1a's 0x811c9dc5)
3. Identify encryption algorithms by their structure (RC4's KSA/PRGA pattern)
4. Pattern matching can help identify winning conditions

## Files

- `tetris` - Challenge binary
- `solve.py` - Solution script
- `gen_flag.cpp` - Flag generator (not distributed)
