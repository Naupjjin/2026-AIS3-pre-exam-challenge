#!/bin/bash

# Build script for Tetris Challenge
# Applies additional obfuscation during compilation

echo "[*] Building Tetris Challenge (Obfuscated + Static Linked)..."

# Compile with:
# -static: static linking to include all libraries
# -fno-inline: prevent function inlining for flattened control flow
# -O1: mild optimizations to preserve obfuscation
# -fno-stack-protector: remove stack canaries
# -fno-exceptions: no exception handling
# -fno-rtti: no runtime type info

g++ -o tetris tetris_obfuscated.cpp \
    -std=c++17 \
    -static \
    -O1 \
    -fno-inline \
    -fno-stack-protector \
    -fno-exceptions \
    -fno-rtti \
    -fno-asynchronous-unwind-tables \
    -fno-unwind-tables \
    -s \
    -Wno-unused-result \
    2>&1

if [ $? -eq 0 ]; then
    echo "[+] Build successful!"
    echo "[*] Stripping debug info..."
    strip --strip-all tetris 2>/dev/null
    strip -R .comment tetris 2>/dev/null
    strip -R .note tetris 2>/dev/null
    strip -R .note.gnu.build-id tetris 2>/dev/null
    strip -R .note.ABI-tag tetris 2>/dev/null
    
    # Additional obfuscation with objcopy if available
    if command -v objcopy &> /dev/null; then
        echo "[*] Applying additional obfuscation..."
        # Rename sections to confuse disassemblers
        objcopy --rename-section .rodata=.r0data tetris 2>/dev/null
        objcopy --rename-section .data=.d4ta tetris 2>/dev/null
        # Add junk section
        echo "DEADBEEFCAFEBABE" | objcopy --add-section .junk=/dev/stdin tetris 2>/dev/null
    fi
    
    echo "[+] Challenge binary ready: tetris"
    echo ""
    echo "File info:"
    file tetris
    ls -lh tetris
    echo ""
    echo "Binary size (static linking makes it larger):"
    du -h tetris
else
    echo "[-] Build failed!"
    exit 1
fi
