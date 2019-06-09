#ifndef _TAPE_H
#define _TAPE_H

#include <sys/types.h>
#include <sys/time.h>


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
    int sems;
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
void incrementSem(tape* tap, int sem_num);
void decrementSem(tape* tap, int sem_num);
int decrementSemNoBlocko(tape* tap, int sem_num);
void getAndPrintPack(tape* tap, truck* truc);
void placePack(tape* tap, pack pac);

#endif