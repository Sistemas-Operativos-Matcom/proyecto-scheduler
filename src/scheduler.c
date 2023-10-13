#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

const int time_lapse_rr = 50;
const int priority_boost_time = 100;

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
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int SJF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int finded = 0;
  int min_pid = procs_info[0].pid;
  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(min_pid) > process_total_time(procs_info[i].pid))
    {
      min_pid = procs_info[i].pid;
    }
    if (procs_info[i].pid == curr_pid)
      finded = 1;
  }

  return finded ? curr_pid : min_pid;
}

int STCF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int min_pid = procs_info[0].pid;
  int min_time = process_total_time(min_pid) - procs_info[0].executed_time;
  for (int i = 0; i < procs_count; i++)
  {
    if (min_time > process_total_time(procs_info[i].pid) - procs_info[i].executed_time)
    {
      min_pid = procs_info[i].pid;
      min_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    }
  }
  return min_pid;
}

int RR_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (procs_count == 0)
    return -1; // por la forma en la que implemente mlfq es necesario esta linea en caso que mlfq le mande a rr un array vacio
  int actual_proc_position;
  int finded = 0;
  for (int i = 0; i < procs_count; i++) // primero buscco la posicion en el array del ultimo proceso que se estaba ejecutando
  {
    if (procs_info[i].pid == curr_pid)
    {
      finded = 1;
      actual_proc_position = i;
    }
  }
  if (finded == 1)
  {
    if (curr_time % time_lapse_rr == 0)
    {
      if (actual_proc_position == procs_count - 1) // si esta en la ultima posicion pues mando el pid de la primera posicion
        return procs_info[0].pid;
      else
        return procs_info[actual_proc_position + 1].pid; // sino pues mando el pid del proceso que esta en la posicion siguiente
    }
    else
      return curr_pid;
  }
  else // en el caso de que ya no este en la lista de procesos pues busco el siguiente pid que le corresponderia ejecutarse
  {
    int min_pid = procs_info[0].pid;
    for (int i = 0; i < procs_count; i++)
    {
      if (procs_info[i].pid < min_pid && procs_info[i].pid > curr_pid)
        min_pid = procs_info[i].pid;
    }
    return min_pid;
  }
}

int MLFQ_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // a partir de aqui usare el executed time del proceso para llevar el tiempo de ejecucion en el cpu
  for (int i = 0; i < procs_count; i++)
  {
    printf("%d\n", *procs_info[i].priority);
    if (procs_info[i].pid == curr_pid)
    {
      if (procs_info[i].on_io == 1)
      {
        procs_info[i].executed_time -= 10; // si el proceso esta en io no le cuento el tiempo de ejecucion
      }
      if (procs_info[i].executed_time > 100 && *procs_info[i].priority < 3) // actualizo la prioridad del proceso si alcanzo el limite de tiempo ejecutado en cpu
      {
        *procs_info[i].priority = *procs_info[i].priority + 1;
        procs_info[i].executed_time = 0;
      }
      break;
    }
  }

  if (curr_time % priority_boost_time == 0) // en base del tiempo de ejecucion activo el priority boost.
  {
    for (int i = 0; i < procs_count; i++)
    {
      *procs_info[i].priority = 1;
      procs_info[i].executed_time = 0;
    }
  }

  int min_priority = *procs_info[0].priority;
  int procs_same_priority = 0;
  // a partir de aqui voy a buscar de todos los procesos cual tiene la prioridad mas alta

  for (int i = 0; i < procs_count; i++)
    min_priority = *procs_info[i].priority < min_priority ? *procs_info[i].priority : min_priority;

  proc_info_t procs_priority[procs_count];

  // ahora solo debo llenar un array con todos los elementos con la prioridad mas alta y pasarselo a rr
  for (int i = 0; i < procs_count; i++)
  {
    if (*procs_info[i].priority == min_priority && procs_info[i].on_io == 0) // solo adiciono los procesos al array si no estan en io
    {
      procs_priority[procs_same_priority] = procs_info[i];
      procs_same_priority++;
    }
  }
  return RR_scheduler(procs_priority, procs_same_priority, curr_time, curr_pid);
}

schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0)
    return *SJF_scheduler;
  if (strcmp(name, "stcf") == 0)
    return *STCF_scheduler;
  if (strcmp(name, "rr") == 0)
    return *RR_scheduler;
  if (strcmp(name, "mlfq") == 0)
    return *MLFQ_scheduler;
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
