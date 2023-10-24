#ifndef QUEUE_H
#define QUEUE_H

#include "simulation.h"

struct Queue{
    int* processes_pid;
    int front,rear,size;
};

void initializeQueue(struct Queue* q,int capacity);

int isEmpty(struct Queue* q);

void enqueue ( int pid, struct Queue* q );

int dequeue ( struct Queue* q );

int find_process( struct Queue* q,int pid);


#endif