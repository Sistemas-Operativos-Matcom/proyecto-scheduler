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
// activos (se asegura que `praocs_count > 0`) y determina el PID del proceso a
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
  
  proc_info_t min = procs_info[0];

  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) < process_total_time(min.pid))
    {
      min =procs_info[i];
    }
  }
    return min.pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  proc_info_t min = procs_info[0];
  int min_time = process_total_time(min.pid)- min.executed_time;

  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time  < min_time)
    { 
      min = procs_info[i];
      min_time = process_total_time(min.pid)- min.executed_time;
    }
  }
    return min.pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

  int proc_pos = 0;

  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid > curr_pid) 
    {
      proc_pos = i;
      break;
    }  
  }
    return procs_info[proc_pos].pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

  if (curr_time % 400 == 0)
  {
    for (int i = 0; i < procs_count; i++)
    {
      *procs_info[i].priority = 0;  
    }   
  }
  
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == curr_pid) 
    {
      *procs_info[i].cpu_time+= 10;
      if (*procs_info[i].cpu_time == 40)
      {
        *procs_info[i].cpu_time = 0;
        *procs_info[i].priority +=1 ;
      }
    }  
  }
  
  int min_priority = 1000000;
  for (int i = 0; i < procs_count; i++)
  {
    if (*procs_info[i].priority < min_priority && procs_info[i].on_io == 0)
    {
      min_priority = *procs_info[i].priority;
    }
  }
  
  if (min_priority == 1000000)
  {
    return rr_scheduler(procs_info, procs_count, curr_time, curr_pid);
  }
  
  proc_info_t *procs_info2= malloc(procs_count*sizeof(proc_info_t));

  int c=0;
  for (int i = 0; i < procs_count; i++)
  {
    if (*procs_info[i].priority==min_priority && procs_info[i].on_io==0)
    {
      procs_info2[c]=procs_info[i];
      c++;
    }
  }

  int pid = rr_scheduler(procs_info2, c, curr_time, curr_pid);
  free(procs_info2);

  return pid;
}

/*int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
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
}*/

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
                                

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
