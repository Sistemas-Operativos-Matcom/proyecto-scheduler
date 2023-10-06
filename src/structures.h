#ifndef STRUCTURES_H
#define STRUCTURES_H
# include "simulation.h"
typedef struct queue {
  int count;
  proc_info_t* arr;
} queue_t;
queue_t* build_queue();
void push(queue_t* q, proc_info_t p);
proc_info_t pop(queue_t* q);
#endif