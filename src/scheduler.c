#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "heap.h"

// La función que define un scheduler está compuesta por los siguientes
// parámetros:
//
//  - procs_info: Array que contiene la información de cada proceso activo
//  - procs_count: Cantidad de procesos activos
//  - curr_time: Tiempo actual de la simulación
//  - curr_pid: PID del proceso que se está ejecutando en el CPU
//
// Esta función se ejecuta en cada timer-interrupt donde existan procesos
// activos (se asegura que `procs_count > 0`) y determina el PID del proceso a
// ejecutar. El valor de retorno es un entero que indica el PID de dicho
// proceso. Pueden ocurrir tres casos:
//
//  - La función devuelve -1: No se ejecuta ningún proceso.
//  - La función devuelve un PID igual al curr_pid: Se mantiene en ejecución el
//  proceso actual.
//  - La función devuelve un PID diferente al curr_pid: Simula un cambio de
//  contexto y se ejecuta el proceso indicado.
//
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sfj_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  int pid = procs_info[0].pid, min_time = process_total_time(procs_info[0].pid), tmp;
  
  for (int i = 0; i < procs_count; i++) {
    if (curr_pid == procs_info[i].pid) return curr_pid;
    
    tmp = process_total_time(procs_info[i].pid);
    if (tmp < min_time) {
      tmp = min_time;
      pid = procs_info[i].pid;
    }
  }
  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  int pid = procs_info[0].pid, min_time = process_total_time(procs_info[0].pid) - procs_info[0].executed_time, tmp;

  for (int i = 0; i < procs_count; i++) {
    tmp = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    if (tmp < min_time) {
      tmp = min_time;
      pid = procs_info[i].pid;
    }
  }
  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  const  int timeskip = 5;
  static int interrupts_elapsed = 0;
  static int curr_pos = 0;
    
  if (timeskip > ++interrupts_elapsed) {
    curr_pos = curr_pos % procs_count;
  }
  else {
    interrupts_elapsed = 0;
    curr_pos = (curr_pos + 1) % procs_count;
  }
  
  return procs_info[curr_pos].pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "rr")   == 0) return *rr_scheduler;
  if (strcmp(name, "sfj")   == 0) return *sfj_scheduler;
  if (strcmp(name, "stcf")   == 0) return *stcf_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
