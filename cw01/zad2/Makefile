CC=gcc
CFLAGS= -std=c99 -Wall
FILE = report2.txt

.PHONY: all clean test

all: main_sh main_st

main_st: main.c libblocks.a
	$(CC) $(CFLAGS) main.c -L. libblocks.a -o main_st

main_sh: main.c libblocks.so
	$(CC) $(CFLAGS) -Wl,-rpath=. -L. main.c libblocks.so -o main_sh 


libblocks.a: blocks_static.o
	ar rcs libblocks.a blocks_static.o 

libblocks.so: blocks_shared.o
	$(CC) -shared -fPIC $(CFLAGS) -o libblocks.so blocks_shared.o

blocks_static.o: blocks.c
	$(CC) $(CFLAGS) -c blocks.c -o blocks_static.o

blocks_shared.o: blocks.c
	$(CC) -fPIC $(CFLAGS) -c blocks.c -o blocks_shared.o

clean: 
	rm -f tmp.txt main_st main_sh *.o *.a *.so

test: all
	> $(FILE)
	make test_st
	make test_sh

test_st:
	echo "-------Static Library----------------------------------------------" >> $(FILE)
	>> $(FILE)
	echo "size: 100000" >> $(FILE)
	echo "-------Big Search-------" >> $(FILE)
	echo "/" >> $(FILE)
	./main_st 100000 -s "/" "*" tmp.txt -d 2 | tee -a $(FILE)
	echo "-------Medium Search-------" >> $(FILE)
	echo "/usr/" >> $(FILE)
	./main_st 100000 -s "/usr/" "*" tmp.txt -d 2 | tee -a $(FILE)
	echo "-------Small Search-------" >> $(FILE)
	echo "/usr/lib" >> $(FILE)
	./main_st 100000 -s "/usr/lib" "*" tmp.txt -d 2 | tee -a $(FILE)
	echo "-------Add and Delete-------" >> $(FILE)
	echo "add and delete: 100000" >> $(FILE)
	./main_st 100000 -a "/etc/issue" 100000 | tee -a $(FILE)
	>> $(FILE)
	
test_sh:
	echo "-------Shared Library----------------------------------------------" >> $(FILE)
	>> $(FILE)
	echo "size: 100000" >> $(FILE)
	echo "-------Big Search-------" >> $(FILE)
	echo "/" >> $(FILE)
	./main_sh 100000 -s "/" "*" tmp.txt -d 2 | tee -a $(FILE)
	echo "-------Medium Search-------" >> $(FILE)
	echo "/usr/" >> $(FILE)
	./main_sh 100000 -s "/usr/" "*" tmp.txt -d 2 | tee -a $(FILE)
	echo "-------Small Search-------" >> $(FILE)
	echo "/usr/lib" >> $(FILE)
	./main_sh 100000 -s "/usr/lib" "*" tmp.txt -d 2 | tee -a $(FILE)
	echo "-------Add and Delete-------" >> $(FILE)
	echo "add and delete: 100000" >> $(FILE)
	./main_sh 100000 -a "/etc/issue" 100000 | tee -a $(FILE)
	>> $(FILE)




