CC = gcc
CFLAGS = -Wall

.PHONY: all test clean

all: main

main: main.c
	$(CC) $(CFLAGS) main.c -o main

test:
	./test.sh > wyniki.txt

clean:
	rm -f main *.o file1 filesys filelib