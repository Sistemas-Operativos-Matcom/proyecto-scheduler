#ifndef QUEUE_H
#define QUEUE_H

#include "simulation.h"
#include <stdlib.h>

typedef struct node {
  int pid;
  struct node *next;
} node_t;

typedef struct queue {
  node_t *head;
  node_t *tail;
} queue_t;

void enqueue(queue_t *queue, int proc_pid);



#endif