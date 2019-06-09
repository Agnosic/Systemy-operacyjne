
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
#include <sys/sem.h>
#include <sys/time.h>

#include "tape.h"
#include "helper.h"


tape* createTape(int maxcount, int maxmass){
    key_t key = ftok(getenv("HOME"), 'x');
    int shm_id = shmget(key, sizeof(tape) + maxcount * sizeof(pack), IPC_CREAT | IPC_EXCL | 0666);
    if(shm_id == -1) ferrno();

    tape* tap = shmat(shm_id, NULL, 0);
    if (tap == (void*) -1){
        shmctl(shm_id, IPC_RMID, NULL);
        ferrno();
    } 

    int sem_id = semget(key, 3, IPC_CREAT | IPC_EXCL | 0666);
    if(sem_id == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        shmdt(tap);
        shmctl(shm_id, IPC_RMID, NULL);
        exit(1);
    }

    tap->first = 0;
    tap->last = 0;
    tap->maxcount = maxcount;
    tap->maxmass = maxmass;
    tap->mass = 0;
    tap->sems = sem_id;
    tap->truckDone = 0;
    tap->empty = 0;
    printf("tape created\n");

    semctl(sem_id, BUFF, SETVAL, 1);
    semctl(sem_id, FILL, SETVAL, 0);
    semctl(sem_id, EMPTY, SETVAL, maxcount);
    return tap;
}

void deleteTape(tape* tap){
    
    semctl(tap->sems, 0, IPC_RMID);
    key_t key = ftok(getenv("HOME"), 'x');
    shmdt(tap);
    shmctl(shmget(key, 0, 0), IPC_RMID, NULL);
}

tape* getTape(){
    key_t key = ftok(getenv("HOME"), 'x');
    int shm_id = shmget(key, 0, 0);
    if(shm_id == -1) ferrno();

    tape* tap = shmat(shm_id, NULL, 0);
    if (tap == (void*) -1) ferrno();
    return tap;
}


pack createPack(int mass){
    pack pac = {getpid(), mass, {}};
    return pac;
}

void placePack(tape* tap, pack pac){
    int done = 0;
    struct timeval start = getCurrTime();
    int firstTime = 1;
    int isTapeFilled = 0;
    int taken;
    while(!done){
        if(tap->truckDone){
            exit(0);
        } 
        if(isTapeFilled == 2){
            printTime(getCurrTime());
            printf(", loader PID: %d. Waiting for tape to load %dkg pack", pac.loader, pac.mass);
        }
        else isTapeFilled = 0;

        if(firstTime){
            firstTime = 0;
            if(!decrementSemNoBlocko(tap, EMPTY)){
                isTapeFilled = 1;
                decrementSem(tap, EMPTY);
            }
        } 
        else {
            decrementSem(tap, EMPTY);;
        }

        decrementSem(tap, BUFF);
        
        if(tap->maxmass >= tap->mass + pac.mass && !tap->truckDone){
            gettimeofday(&pac.loadedTime, NULL);
            tap->buffer[tap->last] = pac;
            tap->last = (tap->last + 1) % tap->maxcount;
            tap->mass += pac.mass;
            taken = (tap->last - tap->first + tap->maxcount - 1)% tap->maxcount + 1;
            done = 1;

            printTime(getCurrTime());
            printf(" , Pack load on tape after %ldus, loader PID %d, pack %dkg. Tape: %d/%d %d/%dkg\n",
                timeDiff(start, getCurrTime()),
                pac.loader,
                pac.mass,
                taken,
                tap->maxcount,
                tap->mass,
                tap->maxmass
            );
        }



        incrementSem(tap,BUFF);
        if(done){
            incrementSem(tap, FILL);
        }
        else{
            incrementSem(tap, EMPTY);
            if(isTapeFilled) isTapeFilled = 2;
        }

    }
}

void getAndPrintPack(tape* tap, truck* truc){
    pack pac;
    int done = 0;
    int firstTime = 1;
    int taken;
    while(!done){
        if(firstTime){
            firstTime = 0;
            if(!decrementSemNoBlocko(tap, FILL)){
                if(tap->truckDone){
                    exit(0);
                } 
                printTime(getCurrTime());
                printf(", Waiting for pack to be put in truck\n");
                decrementSem(tap, FILL);
            }
        } 
        else {
            decrementSem(tap, FILL);;
        }
        if(!tap->truckDone)
            decrementSem(tap, BUFF);
        
        pac = tap->buffer[tap->first];
        if(truc->capacity + pac.mass <= truc->maxCapacity){
            tap->first = (tap->first + 1) % tap->maxcount;
            tap->mass -= pac.mass;
            truc->capacity += pac.mass;
            taken = (tap->last - tap->first + tap->maxcount)% tap->maxcount;
            done = 1;
            printTime(getCurrTime());
            printf(", Pack load on truck after %ldus, loader PID %d, pack %dkg, left in truck %dkg. Tape: %d/%d %d/%dkg\n",
                timeDiff(pac.loadedTime, getCurrTime()),
                pac.loader,
                pac.mass,
                truc->maxCapacity - truc->capacity,
                taken,
                tap->maxcount,
                tap->mass,
                tap->maxmass
            );
        }



        incrementSem(tap,BUFF);
        if(done){
            incrementSem(tap, EMPTY);
        }
        else{
            incrementSem(tap, FILL);
        }

        if(!done){
            printTime(getCurrTime());
            printf(", Filled truck is unloading\n");
            truc->capacity = 0;
            printTime(getCurrTime());
            printf(", New truck arrived\n");
        }
    }


}

void incrementSem(tape* tap, int sem_num){
    struct sembuf sops[1] = {{sem_num, 1, 0}};
    if(semop(tap->sems, sops, 1) == -1){
        ferrno();
    }
}

void decrementSem(tape* tap, int sem_num){
    struct sembuf sops[1] = {{sem_num, -1, 0}};
    if(semop(tap->sems, sops, 1) == -1){
        ferrno();
    }
}

int decrementSemNoBlocko(tape* tap, int sem_num){
        struct sembuf sops[1] = {{sem_num, -1, IPC_NOWAIT}};
    if(semop(tap->sems, sops, 1) == -1){
        if (errno == EAGAIN) return 0;
        else ferrno();
    }
    return 1;

}

