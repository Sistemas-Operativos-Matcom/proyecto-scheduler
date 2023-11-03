#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "simulation.h"

// Devuelve el scheduler implementado según el nombre del mismo
schedule_action_t get_scheduler(const char *name);

typedef struct pair{
  int fs,sc;
} pair_t;

typedef struct queue{
  int Head,Sz,Tail;
  int Arr[500];
}queue_t;

#endif
