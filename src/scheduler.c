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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid)
{
  if (curr_pid != -1)
    return curr_pid; // si hay un proceso ejecutandose entonces se termina de ejecutar
                     //  de lo contario buscamos el proceso con menor tiempo de ejecucion y retornamos su pid
  int min;           // tiempo de ejecucion del proceso
  int result;        // el pid del proceso
  min = process_total_time(procs_info[0].pid);
  result = procs_info[0].pid;

  for (size_t i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) < min)
    {
      min = process_total_time(procs_info[i].pid);
      result = procs_info[i].pid;
    }
  }

  return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  int min;    // tiempo de ejecucion que le queda al proceso
  int result; // el pid del proceso
  min = process_total_time(procs_info[0].pid) - procs_info[0].executed_time;
  result = procs_info[0].pid;

  for (size_t j = 0; j < procs_count; j++) //iteramos buscando el proceso que menos tiempo de ejecucuion le queda
  {
    if (process_total_time(procs_info[j].pid) - procs_info[j].executed_time < min)
    {
      min = process_total_time(procs_info[j].pid) - procs_info[j].executed_time;
      result = procs_info[j].pid;
    }
  }

  return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int position = 0; //variable global en al que guardamos la posicion del proximo proceso a ejecutar
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                 int curr_pid)
{
  if (position >= procs_count) //si la posicion del proceso a ejecutar esta fuera del tamanno del array ejecuto el primer proceso
  {
    position = 1;
    return procs_info[0].pid;
  }
  position = position + 1;
  return procs_info[position - 1].pid;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;

  if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;

  if (strcmp(name, "rr") == 0)
    return *rr_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
