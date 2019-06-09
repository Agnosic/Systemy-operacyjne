#include "queue.h"
#include "helper.h"
#include <stdlib.h>

Queue createQueue(int maxSize){
    Queue queue;
    queue.size = 0;
    queue.maxSize = maxSize;
    queue.tail = 0;
    queue.head = 0;
    queue.data = malloc(sizeof(Train*) * maxSize);
    for(int i = 0; i < maxSize; i++){
        queue.data[i] = NULL;
    }
    return queue;
}

void clearQueue(Queue* queue){
    if(queue->data == NULL) return;
    free(queue->data);
    queue->data = NULL;
    queue->maxSize = -1;
    queue->size = -1;
    free(queue);
}

void push(Queue* queue, Train* train){
    queue->data[queue->tail] = train;
    if(queue->tail >= queue->maxSize){
        queue->tail = 0;
    }
    else{
        queue->tail++;
    }
    queue->size++;
}

Train* pop(Queue* queue){
    Train* train = head(queue);
    if(train){
        queue->data[queue->head] = NULL;
        if(queue->head >= queue->maxSize){
            queue->head = 0;
        }
        else{
            queue->head++;
        }
        queue->size--;
        return train;
    }
    return NULL;
}

Train* head(Queue* queue){
    return queue->data[queue->head];
}

