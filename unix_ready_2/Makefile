CC = gcc
CFLAGS = -Wall -pedantic -ansi -Werror -O2 -g
TARGET = fw
OBJS = main.o fw.o hash.o
TEST_OBJS = test.o fw.o hash.o

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

fw.o: fw.c
	$(CC) $(CFLAGS) -c -o $@ $<

hash.o: hash.c
	$(CC) $(CFLAGS) -c -o $@ $<

test.o: test.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o test $(TEST_OBJS)
	valgrind --quiet --leak-check=full ./test

clean:
	rm -f *.o $(TARGET) test

