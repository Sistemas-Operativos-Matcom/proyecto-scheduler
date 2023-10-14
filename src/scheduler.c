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
#define RR_INTERRUPTS_TO_CHANGE 3
int rr_current_process_interrupts = 0;
int rr_proc_index = 0;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){
  if(curr_pid == -1){
    return procs_info[rr_proc_index%procs_count].pid;
  }

  rr_current_process_interrupts += 1;
  if(rr_current_process_interrupts >= RR_INTERRUPTS_TO_CHANGE){
    rr_current_process_interrupts = 0;
    rr_proc_index = (rr_proc_index+1)%procs_count;
    return procs_info[rr_proc_index].pid;
  }

  return curr_pid;
}

//
// MLFQ
//
#define MLFQ_QUEUE_COUNT 3
#define MLFQ_INTERRUPTS_TO_BOOST 20
#define MLFQ_INTERRUPTS_TO_DECREASE_PRIORITY 6
int mlfq_interrupts_since_boost = 0;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){

  // Asignar prioridad maxima a los procesos recien llegados
  for (int i = 0; i < procs_count; i++){
    if (procs_info[i].executed_time == 0){
      procs_info[i].priority = MLFQ_QUEUE_COUNT-1;
      procs_info[i].executed_interrupts_on_this_priority = 0;
    }
  }

  // Priority boost
  if (mlfq_interrupts_since_boost >= MLFQ_INTERRUPTS_TO_BOOST){
    mlfq_interrupts_since_boost = 0;
    for (int i = 0; i < procs_count; i++){
      procs_info[i].priority = MLFQ_QUEUE_COUNT-1;
      procs_info[i].executed_interrupts_on_this_priority = 0;
    }
  }
  mlfq_interrupts_since_boost += 1;

  // Encontrar la maxima prioridad que tiene un proceso que esta pidiendo tiempo de CPU, y si nadie pide cpu, no ejecutar a nadie
  int max_priority_asking_for_cpu = -1;
  for(int i = 0; i<procs_count; i++){
    int p = procs_info[i].priority;
    if (!procs_info[i].on_io && p > max_priority_asking_for_cpu){ 
      max_priority_asking_for_cpu = p;
    }
  }
  if(max_priority_asking_for_cpu == -1){
    return -1;
  }

  // Determinar si sigo corriendo el proceso actual o hay que cambiarlo
  int change_process_with_rr = 0;
  int rr_displacement = 0;
  if(curr_pid != -1){
    
    // Encontrar el indice del proceso que esta corriendo
    for (int i = 0; i < procs_count; i++){
      if(procs_info[i].pid == curr_pid){
        rr_proc_index = i; 
        break;
      }
    }

    // Actualizar la cantidad de interrupts que ha corrido en esta prioridad, y en el rr
    procs_info[rr_proc_index].executed_interrupts_on_this_priority += 1;
    rr_current_process_interrupts += 1;

    // Ver si tengo que cambiar el proceso que se esta ejecutando segun las reglas del mlfq
    if(procs_info[rr_proc_index].priority < max_priority_asking_for_cpu || procs_info[rr_proc_index].on_io){
      change_process_with_rr = 1;
    }else if(procs_info[rr_proc_index].priority > 0 && procs_info[rr_proc_index].executed_interrupts_on_this_priority >= MLFQ_INTERRUPTS_TO_DECREASE_PRIORITY){
      change_process_with_rr = 1;
      procs_info[rr_proc_index].priority -= 1;
      procs_info[rr_proc_index].executed_interrupts_on_this_priority = 0;
    }else if(rr_current_process_interrupts >= RR_INTERRUPTS_TO_CHANGE){
      change_process_with_rr = 1;
      rr_displacement = 1;
    }
  }else{
    change_process_with_rr = 1;
  }

  // Si se pide, retornar el "siguiente" segun RR
  if(change_process_with_rr){
    for(int i_t = rr_displacement; i_t<procs_count; i_t++){
      int i = (rr_proc_index+i_t)%procs_count;
      if(!procs_info[i].on_io && procs_info[i].priority == max_priority_asking_for_cpu){
        rr_proc_index = i;
        rr_current_process_interrupts = 0;
        break;
      }
    }
  }

  return procs_info[rr_proc_index].pid;
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
