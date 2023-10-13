// #include "mlfq/mlfq.h"
#ifndef AUXILIAR
#include "simulation.h"
#include "mlfq/mlfq.h"

// Removes finished processes from an mlfq according to the list of current active processes
void depurate_mlfq(mlfq _mlfq, proc_info_t *procs_info, int procs_count);

// Verifies if a certain pid is present on a list of current active processes
bool check_existence(proc_info_t *procs_info, int procs_count, int pid);

// get_time delegate
typedef int (*get_time_action)(proc_info_t *);

// Delegate implementations
int sjf_get_time(proc_info_t *process);
int stcf_get_time(proc_info_t *process);

// Returns the shortest job according get_time delegate
int get_shortest_job(proc_info_t *procs_info, int procs_count, get_time_action get_time);

#endif