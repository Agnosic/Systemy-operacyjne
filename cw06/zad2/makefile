CC = gcc
CFLAGS = -Wall -g -lrt

.PHONY: all clean

all: server client

server: server.c
	$(CC) -Wall -o server server.c -lrt

client: client.c
	$(CC) -Wall -o client client.c -lrt

clean:
	rm -f server client *.o 