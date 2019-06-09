#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>
#include <semaphore.h>

typedef struct Train{
    pthread_t *thread_id;
    sem_t *sem;
    int canGetIn;
    int pressed_start;
    pthread_cond_t* changedNumberOfPassangers;
    pthread_cond_t* endOfRide;
    pthread_cond_t* empty;
    pthread_cond_t* start;
}Train;



typedef struct Queue{
    int size;
    int maxSize;
    int head;
    sem_t *sem;
    int tail;
    Train** data;
}Queue;

Queue createQueue(int maxSize);
void clearQueue(Queue* queue);
void push(Queue* queue, Train* train);
Train* pop(Queue* queue);
Train* head(Queue* queue);


#endif