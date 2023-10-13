#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

schedule_action_t get_scheduler(const char *name)
{
    if (strcmp(name, "fifo") == 0)
    {
        return *fifo_scheduler;
    }
    fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
    exit(1);
}
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
    // Se devuelve el PID del primer proceso de todos los disponibles (los
    // procesos están ordenados por orden de llegada).
    return procs_info[0].pid;
}