CC = gcc
CFLAGS = -Wall

.PHONY: all test clean

all: main

main: main.c
	$(CC) $(CFLAGS) main.c -o main

clean:
	rm -f main *.o 