CC = gcc
CFLAGS = -Wall

.PHONY: all test clean

all: monitor tester

monitor: monitor.c
	$(CC) $(CFLAGS) monitor.c -o monitor

tester: tester.c
	$(CC) $(CFLAGS) tester.c -o tester

clean:
	rm -f monitor tester *.o 