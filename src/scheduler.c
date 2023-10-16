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


int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //Variables auxiliares para saber cual es el proceso de menor duracion  .                  
  int shortestPid=-1;
  int shortestDuration=__INT_MAX__;
  int index;

  //Verifico si hay algun proceso siendo ejecutado, en cuyo caso devuelvo su id
  //para que siga siendo ejecutado.
  if(curr_pid != -1)
  {
    return curr_pid;
  }

  //Aqui itero por todos los procesos para quedarme con el de menor duracion total
  for (index=0;index < procs_count; index++)
  {
    int pid = procs_info[index].pid;      
    int on_io = procs_info[index].on_io;  
    int exec_time = procs_info[index].executed_time;              
    int duration = process_total_time(pid);

    if(duration < shortestDuration)
    {
      shortestDuration=duration;
      shortestPid=pid;
    }
  }
  //Devuelvo el de menor duracion (solo si no hay ningun proceso siendo ejecutado)
  return shortestPid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //Variables auxiliares para saber cual es el proceso al que le queda la menor duracion  .
  int shortestPid=-1;
  int shortestDuration=__INT_MAX__;
  int index;

  //Aqui itero por todos los procesos para quedarme con el que le quede la menor duracion.
  for (index=0;index < procs_count; index++)
  {

    int pid = procs_info[index].pid;      
    int on_io = procs_info[index].on_io;  
    int exec_time = procs_info[index].executed_time;              
    int duration = process_total_time(pid);

    //Calculo el tiempo que le queda por ser ejecutado al proceso.
    int remainingT= duration - exec_time;
    if(remainingT< shortestDuration)
    {
      shortestDuration = remainingT;
      shortestPid = pid;
    }
  }
  //Devuelvo el pid del que le queda menor tiempo de ejecucion. 
  return shortestPid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //Variable estatica que declaro para ir variando los procesos               
  static indexProcs=0;
  
  //Como utilizo un slice_time igual al time_interrupt (10s), por cada time_interrupt
  //salto hacia el siguiente proceso. 
  if(indexProcs == procs_count)
  {
    indexProcs=0;
  }
  int currentPid = procs_info[indexProcs].pid; 
  
  indexProcs = (indexProcs + 1) % procs_count;

  //Pid del siguiente proceso
  return currentPid;
}
// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return * fifo_scheduler;
  
  if (strcmp(name, "sjf") == 0) return * sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return * stcf_scheduler;

  if (strcmp(name, "rr") == 0) return * rr_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
