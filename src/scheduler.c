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
                   int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;                 // PID del proceso
  int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
                                               // realizando una opreación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}

const int time_slice = 3 * 10;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                 int curr_pid)
{
  if (curr_pid == -1)
    return procs_info[0].pid;

  if (procs_count == 1)
    return procs_info[0].pid;

  int active_pr_count = 0;

  for (int idx = 0; idx < procs_count; idx++)
  {
    if (procs_info[idx].executed_time <= process_total_time(procs_info[idx].pid))
      active_pr_count += 1;
  }

  proc_info_t *active_process = (proc_info_t *)malloc(active_pr_count * sizeof(proc_info_t));
  int curr = 0;

  for (int idx = 0; idx < procs_count; idx++)
    if (procs_info[idx].executed_time <= process_total_time(procs_info[idx].pid))
      active_process[curr++] = procs_info[idx];

  for (int idx = 0; idx < active_pr_count; idx++)
    if (active_process[idx].pid == curr_pid && active_process[idx].executed_time % time_slice == 0)
      return active_process[(idx + 1) % procs_count].pid;

  // Return same pid if process is still in  it's time slice execution time
  return curr_pid;
}
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  int stcf_pid = -1;
  int stcf_time = __INT_MAX__;
  for (int idx = 0; idx < procs_count; idx++)
  {
    int time_left = process_total_time(procs_info[idx].pid) - procs_info[idx].executed_time;
    if (time_left >= 0 && time_left < stcf_time)
    {
      stcf_time = time_left;
      stcf_pid = procs_info[idx].pid;
    }
  }
  return stcf_pid;
}
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid)
{
  int sjf_pid = -1;
  int sjf_time = __INT_MAX__;
  for (int idx = 0; idx < procs_count; idx++)
  {
    int total_time = process_total_time(procs_info[idx].pid);
    if (total_time < sjf_time)
    {
      sjf_time = total_time;
      sjf_pid = procs_info[idx].pid;
    }
  }
  return sjf_pid;
  // fprintf(stderr, "SJF scheduler Not Implemented ");
  // exit(1);
}
// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:

  if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;
  if (strcmp(name, "rr") == 0)
    return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0)
    return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
