CC = gcc
CFLAGS = -Wall -ggdb -pthread -lm

.PHONY: all clean

all: filtr filtergen

filtr: helper.o helper.h filtr.c
	gcc -Wall -ggdb -pthread    filtr.c helper.o helper.h   -o filtr -lm

filtergen: 

test:
	./test.sh > Times.txt
	rm -f out.pgm

clean:
	rm -f filtr filtergen helper *.o 