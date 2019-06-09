CC = gcc
CFLAGS = -Wall

.PHONY: all clean

all: interpreter

interpreter: interpreter.c
	$(CC) $(CFLAGS) interpreter.c -o interpreter

clean:
	rm -f interpreter *.o 