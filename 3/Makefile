CC = gcc
CFLAGS = -Wall -pedantic -ansi -Werror -g
TARGET = hencode
OBJS = hencode.o huffman.o bitwriter.o

.PHONY: all test clean

all: $(TARGET) hdecode

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

hdecode: hdecode.o huffman.o
	$(CC) $(CFLAGS) -o $@ $^

hdecode.o: hdecode.c
	$(CC) $(CFLAGS) -c -o $@ $<

hencode.o: hencode.c
	$(CC) $(CFLAGS) -c -o $@ $<

huffman.o: huffman.c
	$(CC) $(CFLAGS) -c -o $@ $<

bitwriter.o: bitwriter.c
	$(CC) $(CFLAGS) -c -o $@ $<




clean:
	rm -f *.o $(TARGET) test hdecode

format:
	find . -type f -iname '*.c' -o -iname '*.h' | xargs -I{} clang-format -i -style="{BasedOnStyle: LLVM, ColumnLimit: 80}" {}

