CC = gcc
CFLAGS = -Wall

.PHONY: all clean

all: main_1 main_2

main_1: main_1.c
	$(CC) $(CFLAGS) main_1.c -o main_1

main_2: main_2.c
	$(CC) $(CFLAGS) main_2.c -o main_2


clean:
	rm -f main_1 main_2 *.o 