#!/usr/bin/env python3
"""
Tetris Challenge Solver - AIS3 2026 Pre-Exam
Author: CTF Solver

This script extracts and decrypts the flag from the Tetris reverse engineering challenge.

The challenge uses:
1. Control Flow Flattening with a 256-entry jump table
2. Anti-disassembler techniques (fake instructions, overlapping code)
3. Anti-debug (ptrace, TracerPid, timing checks)
4. RC4 encryption with a key derived from a specific pattern

Key findings from reverse engineering:
- The flag is encrypted and stored in the binary as encFlag[]
- The encryption key is derived from a "WINNING_PATTERN" using FNV-1a hash
- The decryption is RC4-based (symmetric - same algorithm for encrypt/decrypt)
- The WINNING_PATTERN is a 4x10 grid representing colored blocks
"""

# Encrypted flag bytes (extracted from binary)
enc_flag = bytes([
    0x2e, 0xa5, 0x56, 0x46, 0x0d, 0x7c, 0x8e, 0xdc,
    0x83, 0x6f, 0x30, 0x83, 0xff, 0xf8, 0xa5, 0x5c,
    0xd0, 0x76, 0xd8, 0xcd, 0x99, 0xdc, 0x3f, 0x39,
    0x9d, 0x65, 0x70, 0x64
])

BOARD_WIDTH = 10

# The winning pattern (found in binary analysis)
# This represents a specific block arrangement in the first 4 rows
# Colors: 1=Cyan(I), 2=Yellow(O), 3=Magenta(T), 4=Green(S), 5=Red(Z), 6=Blue(J), 7=White(L)
WINNING_PATTERN = [
    [5, 0, 5, 0, 1, 0, 4, 4, 4, 0],  # Row 0
    [5, 5, 5, 0, 1, 0, 4, 0, 0, 0],  # Row 1
    [5, 0, 5, 0, 1, 0, 0, 4, 4, 0],  # Row 2
    [5, 0, 5, 0, 1, 0, 4, 4, 0, 3]   # Row 3
]

def derive_key_from_pattern(pattern):
    """
    Derive 24-byte encryption key from the pattern using FNV-1a hash variant.
    
    The algorithm:
    1. Initialize hash with FNV-1a offset basis (0x811c9dc5)
    2. For each cell in the 4x10 pattern:
       - XOR hash with cell value
       - Multiply by FNV prime (0x01000193)
    3. Generate 24 key bytes using linear congruential generator
    """
    # FNV-1a initialization
    hash_val = 0x811c9dc5
    
    # Process each cell in the pattern
    for i in range(4):
        for j in range(BOARD_WIDTH):
            hash_val ^= pattern[i][j]
            hash_val = (hash_val * 0x01000193) & 0xFFFFFFFF  # Keep as 32-bit
    
    # Generate 24-byte key from hash using LCG
    key = []
    for i in range(24):
        # Extract byte from hash based on position
        byte_val = (hash_val >> ((i % 4) * 8)) & 0xFF
        key.append(byte_val)
        # Update hash using LCG: hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF
        hash_val = (hash_val * 1103515245 + 12345) & 0x7FFFFFFF
    
    return bytes(key)

def rc4_decrypt(data, key):
    """
    RC4 decryption (symmetric - same as encryption).
    
    Standard RC4 implementation:
    1. Key Scheduling Algorithm (KSA) - initialize S-box
    2. Pseudo-Random Generation Algorithm (PRGA) - generate keystream
    """
    # Key Scheduling Algorithm (KSA)
    S = list(range(256))
    j = 0
    for i in range(256):
        j = (j + S[i] + key[i % len(key)]) % 256
        S[i], S[j] = S[j], S[i]
    
    # Pseudo-Random Generation Algorithm (PRGA)
    i = j = 0
    result = []
    for byte in data:
        i = (i + 1) % 256
        j = (j + S[i]) % 256
        S[i], S[j] = S[j], S[i]
        k = S[(S[i] + S[j]) % 256]
        result.append(byte ^ k)
    
    return bytes(result)

def visualize_pattern(pattern):
    """Display the winning pattern as colored blocks."""
    colors = {
        0: '  ',   # Empty
        1: '██',   # Cyan (I)
        2: '██',   # Yellow (O)
        3: '██',   # Magenta (T)
        4: '██',   # Green (S)
        5: '██',   # Red (Z)
        6: '██',   # Blue (J)
        7: '██',   # White (L)
    }
    
    color_codes = {
        0: '',
        1: '\033[96m',   # Cyan
        2: '\033[93m',   # Yellow
        3: '\033[95m',   # Magenta
        4: '\033[92m',   # Green
        5: '\033[91m',   # Red
        6: '\033[94m',   # Blue
        7: '\033[97m',   # White
    }
    
    print("\n[*] Winning Pattern Visualization:")
    print("╔" + "══" * BOARD_WIDTH + "╗")
    for row in pattern:
        print("║", end="")
        for cell in row:
            if cell == 0:
                print("  ", end="")
            else:
                print(f"{color_codes[cell]}██\033[0m", end="")
        print("║")
    print("╚" + "══" * BOARD_WIDTH + "╝")
    
    print("\n[*] Pattern as numbers:")
    for i, row in enumerate(pattern):
        print(f"  Row {i}: {row}")

def main():
    print("=" * 60)
    print("  Tetris Challenge Solver - AIS3 2026 Pre-Exam")
    print("=" * 60)
    
    # Show the winning pattern
    visualize_pattern(WINNING_PATTERN)
    
    # Derive key from pattern
    print("\n[*] Deriving encryption key from pattern...")
    key = derive_key_from_pattern(WINNING_PATTERN)
    print(f"[+] Derived key ({len(key)} bytes):")
    print(f"    {key.hex()}")
    print(f"    Bytes: {[hex(b) for b in key]}")
    
    # Decrypt flag
    print("\n[*] Decrypting flag...")
    print(f"[*] Encrypted flag ({len(enc_flag)} bytes):")
    print(f"    {enc_flag.hex()}")
    
    flag = rc4_decrypt(enc_flag, key)
    
    print("\n" + "=" * 60)
    print(f"[+] FLAG: {flag.decode('utf-8', errors='ignore')}")
    print("=" * 60)
    
    return flag

def extract_from_binary(binary_path):
    """
    Alternative method: Extract encrypted flag directly from binary.
    Useful when you have the binary but not the source code.
    """
    try:
        with open(binary_path, 'rb') as f:
            data = f.read()
        
        # Search for the encrypted flag pattern
        # Looking for the specific byte sequence followed by null terminator
        pattern = bytes([0x2e, 0xa5, 0x56, 0x46])  # First 4 bytes
        
        idx = data.find(pattern)
        if idx != -1:
            print(f"[*] Found encrypted flag at offset 0x{idx:x}")
            # Extract 28 bytes (flag length)
            extracted = data[idx:idx+28]
            print(f"[+] Extracted: {extracted.hex()}")
            return extracted
    except FileNotFoundError:
        print(f"[-] Binary not found: {binary_path}")
    return None

if __name__ == "__main__":
    main()
    
    # Optionally verify against the binary
    print("\n[*] Verifying extraction from binary...")
    extracted = extract_from_binary("tetris")
    if extracted:
        if extracted == enc_flag:
            print("[+] Binary extraction matches hardcoded values!")
        else:
            print("[-] Warning: Extracted bytes differ from hardcoded values")
