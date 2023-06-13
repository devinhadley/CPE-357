CC = gcc
CFLAGS = -Wall -pedantic -g

mush2: mush2.o
	gcc -g -Wall -L ./libmush/lib64 -o mush2 mush2.o -lmush

mush2.o: mush2.c
	gcc -g -c -Wall -I ./libmush/include -o mush2.o mush2.c

all: mush2

format:
	find . -type f -iname '*.c' -o -iname '*.h' | xargs -I{} clang-format -i -style="{BasedOnStyle: LLVM, ColumnLimit: 80}" {}

clean:
	rm ./*.o mush2
