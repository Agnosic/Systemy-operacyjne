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
#include <sys/wait.h>


#include "helper.h"
#include "tape.h"


int main(int argc, char *argv[]){
    if(argc  < 2){
        ferr("pass M and optionally cycle");
    }


    int M = atoi(argv[1]);
    tape* tap = getTape();
    
    pack pac = createPack(M);

    if (argc > 2 ){
        for(int j = 0; j < atoi(argv[2]); j++){
            placePack(tap, pac);
        }
    }
    else{
        while(1){
            placePack(tap, pac);
        }
    }

    return 0;

}



