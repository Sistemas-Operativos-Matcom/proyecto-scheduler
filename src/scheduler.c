#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

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

//
// SJF
//
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){
    if(curr_pid != -1){
      return curr_pid;
    }else{
      // Retornar el de menor tiempo total
      int min_proc_pid = -1;
      int min_proc_time = __INT_MAX__;
      for (int i = 0; i < procs_count; i++){
        int pid = procs_info[i].pid;
        int time = process_total_time(pid);
        if (time < min_proc_time){
          min_proc_pid = pid;
          min_proc_time = time;
        }
      }
      return min_proc_pid;
    }
}

//
// STCF
//
int latest_arrived_pid = 0;
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){

  // Si ha llegado un proceso nuevo 
  if(procs_info[procs_count-1].pid != latest_arrived_pid || curr_pid == -1){
    // Retornar el que le falta menos por ejecutar
    int min_proc_pid = -1;
    int min_proc_time = __INT_MAX__;
    for (int i = 0; i < procs_count; i++){
      int pid = procs_info[i].pid;
      int time = process_total_time(pid)-procs_info[i].executed_time;
      if (time < min_proc_time){
        min_proc_pid = pid;
        min_proc_time = time;
      }
    }
    return min_proc_pid;
  }else{
    return curr_pid;
  }
}

//
// RR
//
#define INTERRUPTS_TO_CHANGE 3
int current_process_interrupts = 0;
int rr_proc_index = 0;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){
  if(curr_pid == -1){
    return procs_info[rr_proc_index%procs_count].pid;
  }

  current_process_interrupts += 1;
  if(current_process_interrupts >= INTERRUPTS_TO_CHANGE){
    current_process_interrupts = 0;
    rr_proc_index = (rr_proc_index+1)%procs_count;
    return procs_info[rr_proc_index].pid;
  }

  return curr_pid;
}

//
// MLFQ
//
#define QUEUE_COUNT 3

#define INTERRUPTS_TO_BOOST 20
int interrupts_since_boost = 0;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){
  return curr_pid; //TODO
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
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
