#ifndef SIMULATION_H
#define SIMULATION_H

#include "process.h"

enum {
  NOT_ARRIVED = 0x1,
  RUNNABLE = 0x2,
  ON_IO = 0x4,
  ENDED = 0x8,
};

typedef struct proc_execution {
  process_t process;
  int pid;
  int executed_time;
  int state;
  int turnaround;
  int response_time;
  int priority;
  int time_in_CPU;
} proc_execution_t;

typedef struct proc_info {
  int pid;
  int executed_time;
  int on_io;
  int *priority;
  int *time_in_CPU;
} proc_info_t;

typedef struct simulation {
  proc_execution_t *procs_exec_info;
  int processes_count;
  int curr_time;
  int curr_proc_pid;
} simulation_t;

typedef int (*schedule_action_t)(proc_info_t *, int, int, int);

// Devuelve el tiempo total que toma la ejecución de un proceso dado su PID
int process_total_time(int pid);

// Comienza una simulación
void start_new_simulation(process_t *processes, int processes_count,
                          schedule_action_t scheduler_action, int config);

#endif
