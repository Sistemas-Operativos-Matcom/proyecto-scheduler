#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  return procs_info[0].pid;
}

int sjf_scheduler (proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {  
  // Se mantiene el proceso 
  if (!(curr_pid) == -1) return curr_pid;
  // Si solo existe un proceso lo manda a ejecutar 
  if (procs_count == 1) return procs_info[0].pid;

  // Instancia de temporales
  int duration = process_total_time(procs_info[0].pid);
  int pid = procs_info[0].pid;

  for (int i = 1; i < procs_count; i++) {
    int duration_temp = process_total_time(procs_info[i].pid);
    if (duration_temp < duration && !(procs_info[i].on_io)) { // Verifica si existe un proceso de menor tiempo
      duration = duration_temp;
      pid = procs_info[i].pid;
    } 
  }

  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // Se tiene que interrumpir el proceso si en la entrada existe un proceso con menor duration
  // Si solo existe un proceso lo manda a ejecutar 
  if (procs_count == 1) return procs_info[0].pid;

  // Instancia de temporales 
  int duration = process_total_time(procs_info[0].pid);
  int pid = procs_info[0].pid;

  for (int i = 1; i < procs_count; i++) {
    int duration_temp = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    if (duration_temp < duration && !(procs_info[i].on_io)) { // Verifica si existe un proceso de menor tiempo
      duration = duration_temp;
      pid = procs_info[i].pid;
    }
  }
  
  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  static int index = -1;
  if (curr_pid == -1) return procs_info[0].pid;
  if (curr_time % 10 != 0) return curr_pid;
  else {
    if (++index >= procs_count) index = 0;
    return procs_info[index].pid;
  }
}

/*
int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  int pid = procs_info[0].pid;      
  int on_io = procs_info[0].on_io;  
  int exec_time = procs_info[0].executed_time; 

  int duration = process_total_time(pid);

  return -1;
}
*/

schedule_action_t get_scheduler(const char *name) {
  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
