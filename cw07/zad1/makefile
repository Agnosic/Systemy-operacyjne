CC = gcc
CFLAGS = -Wall -g

.PHONY: all clean

all: loader trucker worker

loader: loader.o tape.o helper.o

trucker: trucker.o tape.o helper.o

worker: worker.o tape.o helper.o

clean:
	rm -f loader trucker worker *.o 