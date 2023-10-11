#ifndef QUEUE_H
#define QUEUE_H

#include "simulation.h"
#define NUM_QUEUES 3

typedef struct Queue {
    proc_info_t *procs;
    int *executed_time;
    int count;
    int capacity;
} queue_t;

extern queue_t *queues[NUM_QUEUES];

#endif