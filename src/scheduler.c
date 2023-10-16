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
// Se devuelve el PID del primer proceso de todos los disponibles (los
// procesos están ordenados por orden de llegada).
//current devuelve el pid del proceso actual incluso si justo se termino
//el pid esta relacionado con el arrival time (es mayor si se ha demorado en llegar)
//Nombres de Schedulers:
//fifo_scheduler
//sjf
//stcf
//rr
//mlfq

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  return procs_info[0].pid;
}

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
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
//implementacion propia
int sjf(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{ 
  int minProcessTime = __INT_MAX__;
  int nextPid = curr_pid;
  
  for (int i = 0; i < procs_count ; i++)
  {
    if (process_total_time(procs_info[i].pid) < minProcessTime)
    {
      nextPid = procs_info[i].pid;
    }  
  }

  return nextPid;
}

int stcf(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int minTimeToCompletition = process_total_time(procs_info[0].pid) -  procs_info[0].executed_time;
  int nextPid = procs_info[0].pid;
  
  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) -  procs_info[i].executed_time < minTimeToCompletition)
    {
      nextPid = procs_info[i].pid;
    }
  }
  
  return nextPid;
}

int rr(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{ 
  //nextPid se inicializa en el pid del pricmer proceso porque si no tiene sucesor
  //entonces es el ultimo y el siguiente proceso deberia ser el primero en arreglo de procesos
  int nextPid = procs_info[0].pid;
  
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid > curr_pid)
    {
      nextPid = procs_info[i].pid;
      break;
    }
  }
  
  return nextPid;
}
// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  
  if (strcmp(name, "sjf") == 0) return *sjf;
  
  if (strcmp(name, "stcf") == 0) return *stcf;
  
  if (strcmp(name, "rr") == 0) return *rr;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
