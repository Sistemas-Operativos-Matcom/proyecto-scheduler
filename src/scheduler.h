#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "simulation.h"

// Devuelve el scheduler implementado según el nombre del mismo
schedule_action_t get_scheduler(const char *name);

typedef struct process_queue
{
  process_t* processes;
  int count;
} process_queue_t;

typedef struct ml_queue
{
    process_queue_t* process_queue;
    int count;
} ml_queue_t;

#endif
