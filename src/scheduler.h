#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "simulation.h"


// Devuelve el scheduler implementado según el nombre del mismo
schedule_action_t get_scheduler(const char *name, int *time[200]);

#endif
