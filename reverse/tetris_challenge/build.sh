#!/bin/bash

# Build script for Tetris Challenge
# Applies additional obfuscation during compilation

echo "[*] Building Tetris Challenge..."

# Compile with optimizations that help obfuscation
# -fno-inline: prevent function inlining for flattened control flow
# -O2: enable optimizations but preserve control flow
# Strip symbols for harder reversing

g++ -o tetris tetris.cpp \
    -std=c++17 \
    -O2 \
    -fno-inline \
    -fno-stack-protector \
    -s \
    -Wno-unused-result \
    2>&1

if [ $? -eq 0 ]; then
    echo "[+] Build successful!"
    echo "[*] Stripping debug info..."
    strip --strip-all tetris 2>/dev/null
    
    # Additional obfuscation with objcopy if available
    if command -v objcopy &> /dev/null; then
        echo "[*] Applying additional obfuscation..."
        objcopy --rename-section .rodata=.data tetris 2>/dev/null
    fi
    
    echo "[+] Challenge binary ready: tetris"
    echo ""
    echo "File info:"
    file tetris
    ls -la tetris
else
    echo "[-] Build failed!"
    exit 1
fi
