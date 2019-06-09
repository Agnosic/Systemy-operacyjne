#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "helper.h"
#include "queue.h"
#include <time.h>

int capacity;
int train_count;
int passanger_count;
int n;
int ride_count = 0;
int working_train_count = 0;
int train_on_stat = 0;
int start_printed = 0;
int start_count = 0;
int sval;

Train* current_train = NULL;

Queue loadQueue;
Queue endPlatformQueue;

pthread_t *train_thread = NULL;
pthread_t *passanger_thread = NULL;

int* train_ID = NULL;
int* passanger_ID = NULL;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t all_passangers_created_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_ready_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_empty_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t end_platform_queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t load_queue_empty_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t random_passanger_cond = PTHREAD_COND_INITIALIZER;


void init_memory();
void exit_handler();
void init_passangers();
void init_trains();
void* passanger(void* args);
void* train(void* args);
void statement();
Train* createTrain();
void clearTrain(Train* train);
void waitForPassangers();

int main(int argc, char* argv[]){
    srand(time(NULL));
    if(argc < 5){
        ferr("capacity, train count, passanger count, n");
    }
    capacity = atoi(argv[1]);
    working_train_count = train_count = atoi(argv[2]);
    passanger_count = atoi(argv[3]);
    n = atoi(argv[4]);
    if(passanger_count <= capacity * train_count){
        ferr("Too low passangers\n");
    }
    atexit(exit_handler);
  
    init_memory();

    printf("Creating passangers\n");
    init_passangers();

    pthread_mutex_lock(&queue_mutex);
    pthread_cond_wait(&all_passangers_created_cond, &queue_mutex);
    pthread_mutex_unlock(&queue_mutex);

    printf("Creating trains\n");
    init_trains();

    pthread_mutex_lock(&queue_mutex);
    while(working_train_count > 0){
        pthread_cond_wait(&queue_empty_cond, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);

    for(int i = 0; i < passanger_count; i++){
        pthread_join(passanger_thread[i], NULL);
    }

    for(int i = 0; i < train_count; i++){
        pthread_join(train_thread[i], NULL);
    }



    return 0;
}

void init_memory(){
    train_thread = (pthread_t*)malloc(train_count * sizeof(pthread_t));
    passanger_thread = (pthread_t*)malloc(passanger_count * sizeof(pthread_t));

    loadQueue = createQueue(train_count);
    endPlatformQueue = createQueue(train_count);

}

void init_passangers(){
    for(int i = 0; i < passanger_count; i++){
        int* num = malloc(sizeof(int));
        *num = i;
        pthread_create(&passanger_thread[i], NULL, passanger, num);
    }
}

void init_trains(){
    for(int i = 0; i < train_count; i++){
        int* num = malloc(sizeof(int));
        *num = i;
        pthread_create(&train_thread[i], NULL, train, num);
    }
}

void* passanger(void* arg){
    int ride_count = 0;
    pthread_mutex_lock(&queue_mutex);
    if(*(int*)arg == passanger_count - 1){
        printf("Created all passangers\n");
        pthread_cond_broadcast(&all_passangers_created_cond);
    }
    pthread_cond_wait(&queue_ready_cond, &queue_mutex);
    pthread_mutex_unlock(&queue_mutex);
    pthread_mutex_lock(&queue_mutex);
    while(working_train_count > 0){
        if(current_train == NULL){
            current_train = pop(&loadQueue);
            train_on_stat++;
        }
        if(current_train != NULL && current_train->canGetIn && sem_trywait(current_train->sem) != -1 ){
            ride_count++;
            Train *seat = current_train;
            int val;
            sem_getvalue(seat->sem, &val);
            statement();
            printf("Entering train %ld, Amount of passangers in train %d/%d\n", *current_train->thread_id, capacity - val, capacity);
            if(val == 0){
                seat->canGetIn = 0;
                current_train = NULL;
                pthread_cond_broadcast(seat->start);
            }
            pthread_cond_wait(seat->start, &queue_mutex);
            if(seat->pressed_start == 0){
                pthread_cond_broadcast(seat->start);
                seat->pressed_start = 1;
                statement();
                printf("pressed start\n");
                pthread_cond_broadcast(seat->changedNumberOfPassangers);
            }
            pthread_cond_wait(seat->endOfRide, &queue_mutex);
            seat->pressed_start = 0;
            sem_post(seat->sem);
            sem_getvalue(seat->sem, &val);
            statement();
            printf("Exiting train %ld, Amount of passangers in train %d/%d\n", *seat->thread_id, capacity - val, capacity);
            if(val == capacity){
                pthread_cond_broadcast(seat->empty);
                seat->canGetIn = 1;
            }
        }
        if(current_train == NULL && head(&loadQueue) == NULL){
            pthread_cond_signal(&load_queue_empty_cond);
        }


        pthread_mutex_unlock(&queue_mutex);
        pthread_mutex_lock(&queue_mutex);
    }
    free(arg);
    statement();
    printf("I rode %d times\n", ride_count);
    pthread_mutex_unlock(&queue_mutex);
    pthread_exit(0);
}

void* train(void* arg){
    pthread_mutex_lock(&queue_mutex);

    Train* train = createTrain();

    push(&loadQueue, train);
    if(loadQueue.size == loadQueue.maxSize){
        printf("Created all trains\n");
        pthread_cond_broadcast(&queue_ready_cond);
    }
    
    pthread_mutex_unlock(&queue_mutex);
    for(int i = 0; i < n; i++){
        pthread_mutex_lock(&queue_mutex);
        waitForPassangers(train);
        ride_count++;
        statement();
        printf("Door closed\n");
        statement();
        printf("train begins his ride\n");
        start_printed = 0;
        push(&endPlatformQueue, train);
        pthread_mutex_unlock(&queue_mutex);
        int t = rand() % 10;
        usleep(t * 1000);
        pthread_mutex_lock(&queue_mutex);
        statement();
        printf("Ride ended\n");
        while(*head(&endPlatformQueue)->thread_id != pthread_self()){
            pthread_cond_wait(&end_platform_queue_cond, &queue_mutex);
        }
        pthread_cond_wait(&load_queue_empty_cond, &queue_mutex);
        if(i < n - 1){
            push(&loadQueue, train);
        }

        pop(&endPlatformQueue);
        pthread_cond_broadcast(&end_platform_queue_cond);
        statement();
        printf("Door opened\n");
        pthread_cond_broadcast(train->endOfRide);
        pthread_cond_wait(train->empty, &queue_mutex);

        pthread_mutex_unlock(&queue_mutex);
    }
    pthread_mutex_lock(&queue_mutex);
    working_train_count--;
    statement();
    printf("Train ends his ride for good\n");
    if(head(&loadQueue) == NULL)
        pthread_cond_broadcast(&queue_empty_cond);
    pthread_mutex_unlock(&queue_mutex);

    pthread_mutex_lock(&queue_mutex);
    clearTrain(train);
    pthread_mutex_unlock(&queue_mutex);

    pthread_exit(0);
}


void waitForPassangers(Train* train){
    int val;
    sem_getvalue(train->sem, &val);
    while(val > 0){
        pthread_cond_wait(train->changedNumberOfPassangers, &queue_mutex);
        sem_getvalue(train->sem, &val);
    }
}

Train* createTrain(){
    Train* train = malloc(sizeof(Train));
    train->thread_id = malloc(sizeof(pthread_t));
    *train->thread_id = pthread_self();
    train->canGetIn = 1;
    train->sem = malloc(sizeof(sem_t));
    train->changedNumberOfPassangers = malloc(sizeof(pthread_cond_t));
    train->empty = malloc(sizeof(pthread_cond_t));;
    train->endOfRide = malloc(sizeof(pthread_cond_t));
    train->start = malloc(sizeof(pthread_cond_t));
    train->pressed_start = 0;
    pthread_cond_init(train->changedNumberOfPassangers, NULL);
    pthread_cond_init(train->empty, NULL);
    pthread_cond_init(train->start, NULL);
    pthread_cond_init(train->endOfRide, NULL);
    sem_init(train->sem, 0, capacity);
    return train;
}

void clearTrain(Train* train){
    pthread_cond_destroy(train->changedNumberOfPassangers);
    pthread_cond_destroy(train->empty);
    pthread_cond_destroy(train->endOfRide);
    free(train->thread_id);
    free(train->changedNumberOfPassangers);
    free(train->empty);
    free(train->endOfRide);
    sem_destroy(train->sem);
    free(train->sem);
    free(train);
}

void exit_handler(){
    if(passanger_thread != NULL){
        free(passanger_thread);
    }
    if(train_thread != NULL){
        free(train_thread);
    }

}

void statement(){
    print_time(get_curr_time());
    printf(", ID %ld, ", pthread_self());
}

