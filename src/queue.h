#ifndef QUEUE_H
#define QUEUE_H

#include "simulation.h"

typedef struct node
{
    proc_info_t value;
    struct node* next;
}node_t;

typedef struct queue
{
    node_t *head, *tail;
    int len;
}queue_t;

void start_queue(queue_t* q); //se puede usar para limpiar la cola
void push_item(queue_t* q, proc_info_t value);
proc_info_t pop_item(queue_t* q);

#endif