CC = gcc
CFLAGS = -Wall -ggdb

.PHONY: all clean

all: master slave

master: master.c
	$(CC) $(CFLAGS) master.c -o master

slave: slave.c
	$(CC) $(CFLAGS) slave.c -o slave

clean:
	rm -f master slave *.o 