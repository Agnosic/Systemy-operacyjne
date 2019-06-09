//ciezarowka

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>

#include "helper.h"
#include "tape.h"

void handleEXIT();
void handleINT();

tape* tap;

int main(int argc, char *argv[]){
    if(argc != 4){
        ferr("Pass X(truck capacity) K M(tape capacity)");
    }
    atexit(handleEXIT);

    int X = atoi(argv[1]);
    int K = atoi(argv[2]);
    int M = atoi(argv[3]);


    if (signal(SIGINT, handleINT) == SIG_ERR)
        ferrno();

    tap = createTape(K, M);

    truck truc = {0, X};

    while(1){
        getAndPrintPack(tap, &truc);
    }

    return 0;

}

void handleEXIT(){
    if(tap) deleteTape(tap);
}

void handleINT(){
    tap->truckDone = 1;
}

