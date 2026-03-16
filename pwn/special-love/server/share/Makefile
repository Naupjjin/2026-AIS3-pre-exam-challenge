CC = gcc

CFLAGS = \
	-O0 \
	-fstack-protector-strong \
	-fPIE

LDFLAGS = \
	-pie \
	-Wl,-z,relro \
	-Wl,-z,now \
	-Wl,--dynamic-linker=./ld-linux-x86-64.so.2 \
	-Wl,-rpath=.

TARGET = chal

all: $(TARGET)

$(TARGET): chal.c
	$(CC) chal.c -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
