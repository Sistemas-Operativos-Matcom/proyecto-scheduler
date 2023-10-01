#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

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
  int min_pid = procs_info[0].pid;
  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(min_pid) > process_total_time(procs_info[i].pid))
    {
      min_pid = procs_info[i].pid;
    }
  }
  return min_pid;
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
  int actual_proc_position;
  int finded = 0;
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == curr_pid)
    {
      finded = 1;
      actual_proc_position = i;
    }
  }
  if (finded == 1)
  {
    if (actual_proc_position == procs_count - 1)
      return procs_info[0].pid;
    else
      return procs_info[actual_proc_position + 1].pid;
  }
  else
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

  for (int i = 0; i < procs_count; i++) // actualizo si es necesario la prioridad del proceso que se esta ejecutando
  {
    if (procs_info[i].pid == curr_pid)
    {
      if (procs_info[i].executed_time > 50 && procs_info[i].priority < 3)
      {
        procs_info[i].priority++;
      }
      break;
    }
  }

  if (curr_time % priority_boost_time == 0) // en base del tiempo de ejecucion activo el priority boost.
  {
    for (int i = 0; i < procs_count; i++)
    {
      procs_info[i].priority = 1;
    }
  }

  int min_priority = procs_info[0].priority;
  int procs_same_priority = 0;

  for (int i = 0; i < procs_count; i++)
    min_priority = procs_info[i].priority < min_priority ? procs_info[i].priority : min_priority;

  proc_info_t procs_priority[procs_count];
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].priority == min_priority)
    {
      procs_priority[procs_same_priority] = procs_info[i];
      procs_same_priority++;
    }
  }
  return RR_scheduler(procs_priority, procs_same_priority, curr_time, curr_pid);
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
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
