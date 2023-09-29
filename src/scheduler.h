#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "simulation.h"

#define ROUND_ROBIN_TIME_SLICE 50

// Devuelve el scheduler implementado según el nombre del mismo
schedule_action_t get_scheduler(const char *name);

#endif
