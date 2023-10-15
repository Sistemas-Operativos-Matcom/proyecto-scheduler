#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "simulation.h"

//Posicion en el array 'procs_info' que tenia el ultimo proceso que se ejecuto
int pos_last = 0;

//FIFO(First In First Out)
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

//SJF(Shortest Job First)
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(procs_count <= 0)
  {
    return -1;
  }

  int minTime = INT_MAX;
  int pid;
  int duration;
  int pidFinal;

  for(int var=0; var<procs_count; var++)
  {
    pid = procs_info[var].pid;
    duration = process_total_time(pid); 
    if(pid == curr_pid)
    {
      return pid;
    }
    if(duration < minTime)
    {
      minTime = duration;
      pidFinal = pid;
    }
  }

  return pidFinal;
}

//STCF(Shortest Time-to-Completion First)
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(procs_count <= 0)
  {
    return -1;
  }

  int minTimeRemaining = INT_MAX;
  int pid;
  int remaining_time;
  int pidFinal;

  for(int var=0; var<procs_count; var++)
  {
    pid = procs_info[var].pid;
    remaining_time = process_total_time(pid)-procs_info[var].executed_time;
    if(remaining_time < minTimeRemaining)
    {
      minTimeRemaining = remaining_time;
      pidFinal = pid;
    }
  }

  return pidFinal;
}

//RR(Round Robin)
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(procs_count <= 0)
  {
    return -1;
  }
  
  if(curr_pid != -1)
  {
    if(pos_last+1 == procs_count)
    {
      return procs_info[0].pid;
    }

    return procs_info[pos_last+1].pid; 
  }

  return procs_info[pos_last].pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
