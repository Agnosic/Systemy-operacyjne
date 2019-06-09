CC=gcc
CFLAGS= -std=c99 -Wall

.PHONY: all clean

all: libblocks.a libblocks.so

blocks_static.o: blocks.c
	$(CC) $(CFLAGS) -c blocks.c -o blocks_static.o

blocks_shared.o: blocks.c
	$(CC) -fPIC $(CFLAGS) -c blocks.c -o blocks_shared.o

libblocks.a: blocks_static.o
	ar rcs libblocks.a blocks_static.o 

libblocks.so: blocks_shared.o
	$(CC) -shared -fPIC $(CFLAGS) -o libblocks.so blocks_shared.o

clean: 
	rm -f *.o *.a *.so