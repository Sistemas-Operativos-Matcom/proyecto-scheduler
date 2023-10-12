#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

#include "./mlfq/mlfq.h"

#define BOOT_INTERVAL 500

typedef int (*get_time_action)(proc_info_t *);

int sjf_get_time(proc_info_t *process)
{
  return process_total_time(process->pid);
}

int stcf_get_time(proc_info_t *process)
{
  return process_total_time(process->pid) - process->executed_time;
}

// Finds the shortest process among the active ones
int get_shortest_job(proc_info_t *procs_info, int procs_count, get_time_action get_time)
{
  int min_pid = procs_info[0].pid;
  int min_time = get_time(&procs_info[0]);
  // int min_time = process_total_time(min_pid) - procs_info[0].executed_time;

  for (int i = 1; i < procs_count; i++)
  {
    int temp_pid = procs_info[i].pid;
    // int temp_time = process_total_time(temp_pid) - procs_info[i].executed_time;
    int temp_time = get_time(&procs_info[i]);

    if (temp_time < min_time)
    {
      min_time = temp_time;
      min_pid = temp_pid;
    }
  }
  return min_pid;
}

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

// "First-In, First-Out" (FIFO) scheduler
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

// "Shortest Job First" (SJF) scheduler
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return (curr_pid == -1) ? get_shortest_job(procs_info, procs_count, sjf_get_time) : curr_pid;
}

// "Shortest Time-to-Completion First" (STCF) scheduler
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return get_shortest_job(procs_info, procs_count, stcf_get_time);
}

// "Round Robin" (RR) scheduler
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  static const int SLICING_FACTOR = 5;

  // Initializing static variables to keep record of processes execution
  static int process_index = 0, slice_counter = 0;

  process_index = (process_index >= procs_count) ? 0 : process_index;

  slice_counter = (curr_pid == -1) ? 1 : (slice_counter + 1) % SLICING_FACTOR;

  return (slice_counter == 0) ? procs_info[process_index++].pid : procs_info[process_index].pid;
}

static mlfq scheduler;
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Removing complete processes
  if (curr_pid == -1 && scheduler->last_executed_pid != -1)
    mlfq_remove(scheduler, scheduler->last_executed_pid);

  // Boosting processes on proper intervals
  if (curr_time % BOOT_INTERVAL == 0)
    mlfq_boost(scheduler);

  int io_pid = -1;
  for (int i = 0; i < procs_count; i++)
  {
    proc_info_t current_process = procs_info[i];
    // Adding new processes
    if (!mlfq_contains(scheduler, current_process.pid))
      mlfq_add(scheduler, current_process.pid);
    if (current_process.on_io)
      io_pid = current_process.pid;
  }

  // Asumming the time-interval between time-interrupts is 10 milliseconds
  int next_pid = mlfq_get(scheduler, io_pid, 10);
  return next_pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.
  scheduler = new_mlfq();

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  else if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;
  else if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;
  else if (strcmp(name, "rr") == 0)
    return *rr_scheduler;
  else if (strcmp(name, "mlfq") == 0)
    return *mlfq_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}