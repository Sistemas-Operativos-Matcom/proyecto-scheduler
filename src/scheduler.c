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
  int pid = procs_info[0].pid;          //inicializo el pid que devolvere con el pid del primer proceso
  int min_process = process_total_time(pid);  //al igual que con el minimo de los tiempos de ejecucion
  //int size = sizeof(procs_info) / sizeof(procs_info[0]);    //calculo el tamanno del array
  for(int i=1 ; i<procs_count ; i++)               //busco en todo el array el de menor tiempo 
  {
    int new_pid = procs_info[i].pid;
    int new_time = process_total_time(new_pid);
    if(new_time < min_process)
    {
      min_process = new_time;
      pid = new_pid;
    }
  }
  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  int pid = procs_info[0].pid;          //inicializo el pid que devolvere con el pid del primer proceso
  int min_exec_process = process_total_time(pid) - procs_info[0].executed_time;  //al igual que con el minimo de los tiempos que quedan de ejecucion
  //int size = sizeof(procs_info) / sizeof(procs_info[0]);    //calculo el tamanno del array
  for(int i=1 ; i<procs_count ; i++)               //busco en todo el array el de menor tiempo de ejecucion faltante
  {
    int new_pid = procs_info[i].pid;
    int new_exec_time = process_total_time(new_pid) - procs_info[i].executed_time;
    if(new_exec_time < min_exec_process)
    {
      min_exec_process = new_exec_time;
      pid = new_pid;
    }
  }
  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(curr_time % 5 == 0)
  int pid;
  for(int i=0 ; i<procs_count ; i++)
  {
    if(curr_pid == procs_info[i].pid)
    {
      if(i==procs_count-1) pid = procs_info[0];
      else pid = procs_info[i+1];
      break;
    }
  }
  return pid;
}


// Información que puedes obtener de un proceso
  //int pid = procs_info[0].pid;      // PID del proceso
  //int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  //int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  //int duration = process_total_time(pid);


// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
