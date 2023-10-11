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



int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles
  return procs_info[0].pid;
}


int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  // Se devuelve el PID del proceso con menor tamaño
  int id=0;
  int PID=procs_info[0].pid;
  for(int i=0;i<procs_count;i++){
    if(process_total_time(procs_info[i].pid)<process_total_time(procs_info[id].pid)){
      id=i;
      PID=procs_info[i].pid;
    }
  } 
  return PID;
} 

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  // Se devuelve el PID con el menor tiempo a ser completado.
  int id=0; 
  int PID=procs_info[0].pid;
  for(int i=0;i<procs_count;i++){
    if(process_total_time(procs_info[i].pid)-procs_info[i].executed_time < process_total_time(procs_info[id].pid)-procs_info[id].executed_time){
      id=i;
      PID=procs_info[i].pid;
    }
  } 
  return PID;
} 

#define QUANTUM 5
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    // Round Robin, en todo su esplendor 
    static int prev_change = 0; 
    static int id= 0; 

    if( curr_pid==-1 || curr_time-prev_change >= QUANTUM) {
        id= (id+1) % procs_count; 
        prev_change = curr_time; 
        return procs_info[id].pid;
    }
    return curr_pid;
}


schedule_action_t get_scheduler(const char *name) {

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
