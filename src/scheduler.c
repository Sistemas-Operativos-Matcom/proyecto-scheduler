#include "scheduler.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

// La función que define un scheduler está compuesta por los siguientes
// parámetros:
//
//!  - procs_info: Array que contiene la información de cada proceso activo
//!  - procs_count: Cantidad de procesos activos
//!  - curr_time: Tiempo actual de la simulación
//!  - curr_pid: PID del proceso que se está ejecutando en el CPU
//
//* Esta función se ejecuta en cada timer-interrupt donde existan procesos
//*   activos (se asegura que `procs_count > 0`) y determina el PID del proceso a
//*   ejecutar. El valor de retorno es un entero que indica el PID de dicho
//*   proceso. Pueden ocurrir tres casos:
//
//* - La función devuelve -1: No se ejecuta ningún proceso.

//* - La función devuelve un PID igual al curr_pid: Se mantiene en ejecución el
//*     proceso actual.

//* - La función devuelve un PID diferente al curr_pid: Simula un cambio de
//*     contexto y se ejecuta el proceso indicado.

static int latest_sj = -2;
static int latest_rr = -2;
static int last_index_rr = 0;

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{  
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{ 
  if(curr_pid == latest_sj) return curr_pid;
 
  int shortest_job = INT_MAX;
  int shortest_job_pid = 0;
  for (size_t i = 0; i < procs_count; i++)
  { 
    if (process_total_time(procs_info[i].pid) < shortest_job)
    {
      shortest_job = process_total_time(procs_info[i].pid);
      shortest_job_pid = procs_info[i].pid;
    }
  }
  latest_sj = shortest_job_pid;
  return shortest_job_pid; 
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  int shortest_job = INT_MAX;
  int shortest_job_pid = 0;
  for (size_t i = 0; i < procs_count; i++)
  { 
    if (process_total_time(procs_info[i].pid) < shortest_job)
    {
      shortest_job = process_total_time(procs_info[i].pid);
      shortest_job_pid = procs_info[i].pid;
    }
  }
  latest_sj = shortest_job_pid;
  return shortest_job_pid; 
}

int roundRobin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
    //time slice = time interrupt
    if(last_index_rr < procs_count - 1)
    {
      last_index_rr += 1;
      latest_rr = procs_info[last_index_rr].pid;
      return procs_info[last_index_rr].pid;
    }
    else
    {
      last_index_rr = 0;
      latest_rr = procs_info[last_index_rr].pid;
      return procs_info[last_index_rr].pid;
    }
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  
  int pid = procs_info[0].pid;
  int on_io = procs_info[0].on_io;  
  int exec_time = procs_info[0].executed_time;
  int duration = process_total_time(pid);

  return -1;
}

//* Esta función devuelve la función que se ejecutará en cada timer-interrupt
//*   según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *roundRobin_scheduler;

  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
