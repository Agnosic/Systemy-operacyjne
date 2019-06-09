CC = gcc
CFLAGS = -Wall -g

.PHONY: all test clean

all: monitor

monitor: monitor.c
	$(CC) $(CFLAGS) monitor.c -o monitor



clean:
	rm -f monitor *.o 