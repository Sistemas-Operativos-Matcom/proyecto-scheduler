#ifndef QUEUE_H
#define QUEUE_H

#include "simulation.h"

struct Queue{
    proc_info_t *processes[200];
    int front,rear;
};

void enqueue ( proc_info_t *process, struct Queue *queue );

void dequeue ( proc_info_t *process, struct Queue *queue );


#endif