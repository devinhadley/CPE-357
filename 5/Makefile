CC = gcc
CFLAGS = -Wall -pedantic -g

all: mytalk

mytalk: shared.o client.o server.o mytalk.o
	$(CC) $(CFLAGS) -L ./Talk/lib64 -o mytalk shared.o server.o client.o mytalk.o -ltalk -lncurses

mytalk.o: mytalk.c
	$(CC) $(CFLAGS) -c -I ~./Talk/include mytalk.c

server.o: server.c
	$(CC) $(CFLAGS) -c -I ~./Talk/include server.c

client.o: client.c
	$(CC) $(CFLAGS) -c -I ~./Talk/include client.c

shared.o: shared.c
	$(CC) $(CFLAGS) -c -I ~./Talk/include shared.c

clean:
	rm *.o mytalk

upload:
	upload_folder ~/357/asgns/5



clean:
