#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

#define TIME_SLICE 50

static process_queue_t *l_queue;

static process_queue_t new_queue(process_t *processes, int process_count){
  process_queue_t queue = {
    (process_t *)malloc(process_count * (sizeof (process_t))),
    process_count
  };

  return queue;
}

static ml_queue_t new_ml_queue(process_t *processes, int process_count, int levels){
  ml_queue_t ml_queue = {
    (process_queue_t *)malloc(levels * (sizeof (process_queue_t))),
    levels,
  };

  for(int i = 0; i<levels; i++){
    ml_queue.process_queue[i] = new_queue(processes, process_count);
  }

  
}


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

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

  int ld = __INT_MAX__;
  int pid = curr_pid;

  if(process_total_time(pid) == 0){

    for(int i = 0; i < procs_count; i++){

      int proc_pid = procs_info[i].pid;
      int duration = process_total_time(proc_pid);
      
      if(duration < ld && duration > 0){
        pid = proc_pid;
        ld = duration;
      }
    }
  }

  
  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int st = __INT_MAX__;
  int pid = curr_pid;

  for(int i = 0; i < procs_count; i++){

    int proc_pid = procs_info[i].pid;
    int t_exc = process_total_time(proc_pid) - procs_info[i].executed_time;
    
    if(t_exc < st){
      pid = proc_pid;
      st = t_exc;
    }
  }
  
  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int pid = curr_pid;

  if(curr_time%TIME_SLICE == 0){

    for(int i = 0; i< procs_count; i++){
      if(procs_info[i].pid == curr_pid){
        pid = procs_info[(i+1)%procs_count].pid;
      }
    }

    if(pid == curr_pid) return procs_info[0].pid;
  }

  return pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
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
