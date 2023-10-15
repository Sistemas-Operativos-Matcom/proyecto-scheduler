#include "scheduler.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
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
static int latest_lj = -2;
static int latest_rr = -2;
static int last_index_rr = 0;

//improved rr
static bool in_boost = false;
static int rr_slice = 100;
static int rr_boost = 1000;
static int rr_slice_boost = 80;
static int previous_to_boost = 0;

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
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < shortest_job)
    {
      shortest_job = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      shortest_job_pid = procs_info[i].pid;
    }
  }
  latest_sj = shortest_job_pid;
  return shortest_job_pid; 
}

//experimental scheduler longest job first
int ltcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  int shortest_job = INT_MIN;
  int shortest_job_pid = 0;
  for (size_t i = 0; i < procs_count; i++)
  { 
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time > shortest_job)
    {
      shortest_job = process_total_time(procs_info[i].pid);
      shortest_job_pid = procs_info[i].pid;
    }
  }
  latest_lj = shortest_job_pid;
  return shortest_job_pid; 
}

//and indexer for the last function (lazy)
int ltcf_indexer(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  int longest_job = INT_MIN;
  int longest_job_index = 0;
  for (size_t i = 0; i < procs_count; i++)
  { 
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time > longest_job)
    {
      longest_job_index = i;
    }
  }
  return longest_job_index; 
}

int stcf_indexer(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  int shortest = INT_MAX;
  int shortest_index = 0;
  for (size_t i = 0; i < procs_count; i++)
  { 
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < shortest)
    {
      shortest_index = i;
    }
  }
  return shortest_index; 
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

//an improved rr with a mini boost to avoid starvation and a bigger time slice
int rrImproved_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
    if(in_boost)
    {
      if(rr_slice_boost != 0 && curr_pid != -1)
      {
        rr_slice_boost -= 10;
        rr_boost -= 10;
        return procs_info[last_index_rr].pid;
      }
      else
      {
        rr_slice_boost = 80;
        in_boost = false;
        last_index_rr = previous_to_boost;
      }
    }

    //mini boost for the slowest/longest, it improves response time but remains costly in turnaround.
    if(rr_boost == 0)
    {
      in_boost = true;
      previous_to_boost = last_index_rr;
      last_index_rr = stcf_indexer(procs_info, procs_count, curr_time, curr_pid); //longest
      latest_rr = procs_info[last_index_rr].pid;
      return latest_rr;
    }

    if (rr_slice != 0 && curr_pid != -1)
    {
      rr_boost -= 10;
      rr_slice -= 10;
      return procs_info[last_index_rr].pid;
    }
    else
    {
      rr_slice = 100;
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
}


//* Esta función devuelve la función que se ejecutará en cada timer-interrupt
//*   según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "ltcf") == 0) return *ltcf_scheduler;

  if (strcmp(name, "rr") == 0) return *roundRobin_scheduler;

  if (strcmp(name, "rri") == 0) return *rrImproved_scheduler;


  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
