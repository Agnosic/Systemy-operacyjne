#ifndef _TAPE_H
#define _TAPE_H

#include <sys/types.h>
#include <sys/time.h>
#include <semaphore.h>


typedef struct pack{
    pid_t loader;
    int mass;
    struct timeval loadedTime;
}pack;

typedef struct truck{
    int capacity;
    int maxCapacity;
}truck;

typedef struct tape{
    int first;
    int last;
    int maxcount;
    int maxmass;
    int mass;
    int truckDone;
    int empty;
    pack buffer[];
}tape;

enum sem {
    BUFF = 0,
    EMPTY = 1,
    FILL = 2
};



tape* createTape(int maxcount, int maxmass);
void deleteTape(tape* tap);
tape* getTape();
pack createPack(int mass);
void incrementSem(int sem_num);
void decrementSem(int sem_num);
int decrementSemNoBlocko(int sem_num);
void getAndPrintPack(tape* tap, truck* truc);
void placePack(tape* tap, pack pac);


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
#include <semaphore.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "tape.h"
#include "helper.h"

sem_t* semap[3];
tape* tap;
long size;

tape* createTape(int maxcount, int maxmass){
    int shm_id = shm_open("/tape", O_RDWR | O_CREAT | O_EXCL, 0666);
    if(shm_id == -1) ferrno();

    size = sizeof(tape) + maxcount*sizeof(pack);
    if(ftruncate(shm_id, sizeof(tape) + maxcount*sizeof(pack)) == -1)
        ferrno();
    
    tap = mmap(NULL, sizeof(tape) + maxcount*sizeof(pack), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

    close(shm_id);
    if(tap == (void *) -1) ferrno();



    tap->first = 0;
    tap->last = 0;
    tap->maxcount = maxcount;
    tap->maxmass = maxmass;
    tap->mass = 0;
    tap->truckDone = 0;
    tap->empty = 0;
    printf("tape created\n");

    if((semap[BUFF] = sem_open("/buff", O_RDWR | O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) ferrno();
    if((semap[FILL] = sem_open("/fill", O_RDWR | O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) ferrno();
    if((semap[EMPTY] = sem_open("/empty", O_RDWR | O_CREAT | O_EXCL, 0666, maxcount)) == SEM_FAILED) ferrno();
    return tap;
}

void deleteTape(tape* tap){
    
    sem_close(semap[BUFF]);
    sem_close(semap[FILL]);
    sem_close(semap[EMPTY]);
    sem_unlink("/buff");
    sem_unlink("/fill");
    sem_unlink("/empty");
    munmap(tap, size);
    if(shm_unlink("/tape")){
        printf("unlinked");
    }

}

tape* getTape(){
    int shm_id = shm_open("/tape", O_RDWR, 0);//
    int size;
    if(shm_id == -1) ferrno();
    read(shm_id, &size, sizeof(int));

    tap = mmap(NULL, sizeof(tape) + size*sizeof(pack), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

    close(shm_id);
    if (tap == (void*) -1) ferrno();

     if((semap[BUFF] = sem_open("/buff", O_RDWR)) == SEM_FAILED) ferrno();
    if(( semap[FILL] = sem_open("/fill", O_RDWR)) == SEM_FAILED) ferrno();
    if(( semap[EMPTY] = sem_open("/empty", O_RDWR)) == SEM_FAILED) ferrno();
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
            sem_close(semap[BUFF]);
            sem_close(semap[FILL]);
            sem_close(semap[EMPTY]);
            munmap(tap, size);
            exit(0);
        } 
        if(isTapeFilled == 2){
            printTime(getCurrTime());
            printf(", loader PID: %d. Waiting for tape to load %dkg pack", pac.loader, pac.mass);
        }
        else isTapeFilled = 0;

        if(firstTime){
            firstTime = 0;
            if(!decrementSemNoBlocko(EMPTY)){
                isTapeFilled = 1;
                decrementSem(EMPTY);
            }
        } 
        else {
            decrementSem(EMPTY);;
        }

        decrementSem(BUFF);
        
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



        incrementSem(BUFF);
        if(done){
            incrementSem(FILL);
        }
        else{
            incrementSem(EMPTY);
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
            if(!decrementSemNoBlocko(FILL)){
                if(tap->truckDone){
                    exit(0);
                } 
                printTime(getCurrTime());
                printf(", Waiting for pack to be put in truck\n");
                decrementSem(FILL);
            }
        } 
        else {
            decrementSem(FILL);;
        }
        decrementSem(BUFF);
        
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



        incrementSem(BUFF);
        if(done){
            incrementSem(EMPTY);
        }
        else{
            incrementSem(FILL);
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

void incrementSem(int sem_num){
    if(sem_post(semap[sem_num]) == -1){
        ferrno();
    }
}

void decrementSem(int sem_num){
    if(sem_wait(semap[sem_num]) == -1){
        ferrno();
    }
}

int decrementSemNoBlocko(int sem_num){

    if(sem_trywait(semap[sem_num]) == -1){
        if (errno == EAGAIN) return 0;
        else ferrno();
    }
    return 1;

}



#endif