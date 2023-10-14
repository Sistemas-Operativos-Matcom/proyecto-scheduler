#ifndef QUEUE_H
#define QUEUE_H

#include "scheduler.h"
#include "simulation.h"
typedef struct Queue
{
    proc_info_t items[__INT16_MAX__];
    int pop;
    int puntero;
} Queue;
void initQueue(Queue *q);
int isFull(Queue *q);
int length(struct Queue *q);
int isEmpty(Queue *q);
void enqueue(Queue *q, proc_info_t value);
proc_info_t dequeue(Queue *q);
int contains(struct Queue *q, proc_info_t item);
#endif
