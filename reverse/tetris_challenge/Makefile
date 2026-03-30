# Tetris Challenge - AIS3 Pre-Exam 2026

CXX = g++
CXXFLAGS = -fno-stack-protector -no-pie
STRIP = strip

TARGET = tetris

.PHONY: all clean

all: $(TARGET)

$(TARGET): tetris.cpp
	@echo "[*] Building tetris binary (100000 junk functions, may take a while)..."
	$(CXX) $(CXXFLAGS) -o $@ $<
	$(STRIP) $@
	@echo "[+] Done: $@"

clean:
	rm -f $(TARGET)
