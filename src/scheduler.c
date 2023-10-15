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
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

// Shortest Job First (SJF)
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // Se devuelve el PID del proceso con menor duración de todos los disponibles

  int shortest_job_pid=procs_info[0].pid;
  int shortest_job_duration = process_total_time(shortest_job_pid);
  for(int i = 1; i < procs_count; i++){
    int pid = procs_info[i].pid;
    int duration = process_total_time(pid);
    if(shortest_job_duration > duration){
      shortest_job_duration = duration;
      shortest_job_pid = pid;
    }
  }

  return shortest_job_pid;
}

// Shortest Time-to-Completion First (STCF)
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // Se devuelve el PID del proceso con menor duración de todos los disponibles

  int stc_pid=procs_info[0].pid;
  int stc_rem_time = process_total_time(stc_pid) - procs_info[0].executed_time;
  for(int i = 1; i < procs_count; i++){
    int pid = procs_info[i].pid;
    int duration = process_total_time(pid);
    int exec_time = procs_info[i].executed_time;
    int rem_time = duration - exec_time;
    if(stc_rem_time > rem_time){
      stc_rem_time = rem_time;
      stc_pid = pid;
    }
  }

  return stc_pid;
}

// Round Robin (RR)
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){
  // Cambia de proceso cada cierto tiempo (time slice)

  static int time_slice_counter = 0; // para simular el time_slice = timer_interrupt * 2
  static int proc_index = 0;
  if (curr_pid != -1 && time_slice_counter < 2){
    time_slice_counter++;
    return curr_pid;
  }
  time_slice_counter = 0;
  proc_index = (proc_index + 1) % procs_count;
  return procs_info[proc_index].pid;
}

#define MAX_PROCS 100005
#define NUM_QUEUES 3
#define TIME_SLICES {5, 10, 20} // Time slice para cada cola
#define BOOST_INTERVAL 10 // Intervalo de tiempo después del cual se "boostean" las prioridades

int queues[NUM_QUEUES][MAX_PROCS];
int time_slice_counters[MAX_PROCS];
int queue_heads[NUM_QUEUES] = {0};
int queue_tails[NUM_QUEUES] = {0};
int seen[MAX_PROCS];
int mlfq_init = 0;

// Inicializacion del MLFQ
void init_mlfq(proc_info_t *procs_info, int procs_count) {
    for (int i = 0; i < procs_count; i++) {
        queues[0][queue_tails[0]++] = procs_info[i].pid;
        seen[procs_info[i].pid] = 1;
    }
}

// Multi-level Feedback Queue (MLFQ)
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    if(mlfq_init == 0){
      // inicializar
      init_mlfq(procs_info,procs_count);
      mlfq_init = 1;
    }

    // insertar nuevos procesos en la cola de mayor prioridad
    for(int i = procs_count - 1; i >= 0; i--){
      if(seen[procs_info[i].pid] == 0){
        queues[0][queue_tails[0]++] = procs_info[i].pid;
        seen[procs_info[i].pid] = 1;
      }
      else{
        break;
      }
    }

    // Priority Boost
    if (curr_time % BOOST_INTERVAL == 0) {
      for (int q = 1; q < NUM_QUEUES; q++) {
        for(int j = queue_heads[q]; j < queue_tails[q]; j++){
          queues[0][queue_tails[0]++] = queues[q][j];
        }
        queue_heads[q] = queue_tails[q] = 0;
      }
    }

    int chosen_pid = -1;
    int chosen_queue = -1;

    // buscar proceso de mayor prioridad
    for (int q = 0; q < NUM_QUEUES; q++) {
      if (queue_heads[q] != queue_tails[q]) {
        chosen_pid = queues[q][queue_heads[q]];
        int ok = 0;
        for(int j = 0; j < procs_count; j++){
          if(procs_info[j].pid == chosen_pid){
            ok = 1;
            break;
          }
        }
        queue_heads[q]++;
        if(ok == 0){
          q--;
          continue;
        }
        chosen_queue = q;
        break;
      }
    }

    if (chosen_pid != -1) {
      if (chosen_queue < NUM_QUEUES - 1) {
        queues[chosen_queue][queue_tails[chosen_queue]++] = chosen_pid;
      }
    }

    return chosen_pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
