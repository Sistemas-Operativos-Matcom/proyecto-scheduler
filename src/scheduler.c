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

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  int ans_pid = procs_info[0].pid;
  int ans_duration = process_total_time(ans_pid);

  if(curr_pid == -1){

    for (int i = 1; i < procs_count; i++){
      int i_pid = procs_info[i].pid;
      int i_duration = process_total_time(i_pid);

      if(i_duration < ans_duration){
        ans_pid = i_pid;
        ans_duration = i_duration;
      }
    }
  }
  else {
    ans_pid = curr_pid;
  }
  return ans_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  int ans_pid = procs_info[0].pid;
  int exec_time = procs_info[0].executed_time;
  int duration = process_total_time(ans_pid);

  int ans_time_to_completion = duration - exec_time;

  for (int i = 1; i < procs_count; i++){

    int i_pid = procs_info[i].pid;
    int i_exec_time = procs_info[i].executed_time;
    int i_duration = process_total_time(i_pid);
    int i_time_to_completion = i_duration - i_exec_time;

    if(ans_time_to_completion > i_time_to_completion){
      ans_pid = i_pid;
      ans_time_to_completion = i_time_to_completion;
    }
  }
  return ans_pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  int time_slice = 100;

  if(curr_time % time_slice == 0 || curr_pid == -1){
    if(curr_pid == procs_info[procs_count - 1].pid || curr_pid == -1){ return procs_info[0].pid; }
    else{
      for(int i = 0; i < procs_count; i++){
        if(procs_info[i].pid == curr_pid){
          return procs_info[i+1].pid;
        }
      }
    }
  }
  return curr_pid;
}
/*
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  // int pid = procs_info[0].pid; // PID del proceso
  //int on_io = procs_info[0].on_io; // Indica si el proceso se encuentra realizando una opreación IO
  // int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha ejecutado (en CPU o en I/O)
  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  // int duration = process_total_time(pid);

  *q_0 =

}*/

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

//   sjf_scheduler
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

//  stcf_scheduler
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

//  rr_scheduler
  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
