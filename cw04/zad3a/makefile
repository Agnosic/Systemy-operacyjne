CC = gcc
CFLAGS = -Wall -g

.PHONY: all test clean

all: sender catcher

sender: sender.c
	$(CC) $(CFLAGS) sender.c -o sender

catcher: catcher.c
	$(CC) $(CFLAGS) catcher.c -o catcher



clean:
	rm -f sender catcher *.o 