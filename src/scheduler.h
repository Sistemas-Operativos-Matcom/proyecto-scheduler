#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "simulation.h"

#define ROUND_ROBIN_TIME_SLICE 50
#define MLFQ_PRIORITY_BOOST_TIME 2000
#define PRIORITY_LEVELS 3


// Devuelve el scheduler implementado según el nombre del mismo
schedule_action_t get_scheduler(const char *name);

// Feedback queue type definitiion that stores processes ids
typedef struct feedback_queue {
    int *procs;
    int size;
    int procs_count;
} fq_t;

#endif