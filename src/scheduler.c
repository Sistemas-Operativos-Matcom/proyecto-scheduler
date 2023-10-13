#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "auxiliar.h"

#define BOOT_INTERVAL 500

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
  int next_pid = (curr_pid == -1) ? get_shortest_job(procs_info, procs_count, sjf_get_time) : curr_pid;
  if (!check_existence(procs_info, procs_count, next_pid))
    return sjf_scheduler(procs_info, procs_count, curr_time, -1);
  return next_pid;
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
  // Removing already finished processes from mlfq
  depurate_mlfq(scheduler, procs_info, procs_count);

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
    // Storing current io process
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