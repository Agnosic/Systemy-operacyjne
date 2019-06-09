CC = gcc
CFLAGS = -Wall -lpthread -lrt

.PHONY: all clean

all: loader worker trucker

loader: loader.c tape.h
	$(CC) $< -o $@ $(CFLAGS)

trucker: trucker.c tape.h
	$(CC) $< -o $@ $(CFLAGS)

worker: worker.c tape.h
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f loader trucker worker *.o 