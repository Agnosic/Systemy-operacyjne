#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>


#include "helper.h"
#include "tape.h"

void handleEXIT();
void handleINT();

int worker_count;

int main(int argc, char *argv[]){
    if(argc  < 2){
        ferr("Pass worker count their mass and optionally their cycle");
    }


    worker_count = atoi(argv[1]);

    for(int i = 0; i < worker_count; i++){
        if(fork() == 0){
            if (signal(SIGINT, handleINT) == SIG_ERR)
                ferrno();
            

            if (argc > 2 + worker_count){
                for(int j = 0; j < atoi(argv[2+worker_count]); j++){
                    execl("./worker", "./worker", argv[2+i], argv[2+worker_count], NULL);
                }
            }
            else{
                while(1){
                    execl("./worker", "./worker", argv[2+i], NULL);
                }
            }
        }
    }

    for (int i = 0; i < worker_count; i++){
        wait(NULL);
    }

    return 0;

}



void handleINT(){
    for (int i = 0; i < worker_count; i++){
        wait(NULL);
    }
    exit(0);
}