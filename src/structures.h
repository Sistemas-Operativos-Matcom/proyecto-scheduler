#ifndef STRUCTURES_H
#define STRUCTURES_H
# include "simulation.h"
typedef struct queue {
  int count;
  int* arr;
} queue_t;
queue_t* build_queue();
void push(queue_t* q, int p);
int pop(queue_t* q);
#endif