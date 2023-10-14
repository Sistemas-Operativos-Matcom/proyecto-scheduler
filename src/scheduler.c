#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int actual_time_interrupt_count_to_time_slice = 0;
int round_robin_process_counter = 0;
int Time_Slice = 50;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SJF(proc_info_t *procs_info, int procs_count, int curr_time,
        int curr_pid)
{

  if (curr_pid != -1) // : si no has terminado de ejecutarte,mantente ejecutandote
  {
    return curr_pid;
  }

  int min_total_time = process_total_time(procs_info[0].pid); // : minimo tiempo total de ejecucion
  int next_procs_pid = procs_info[0].pid;
  for (int i = 0; i < procs_count; i++)
  {
    if (min_total_time > process_total_time(procs_info[i].pid))
    {
      next_procs_pid = procs_info[i].pid;
      min_total_time = process_total_time(procs_info[i].pid); // : como no hay nadie ejecutandose,quedate con el q menos tiempo total demore en ejecucion
    }
  }

  return next_procs_pid;

  /*// Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;                 // PID del proceso
  int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
                                               // realizando una opreación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int STCF(proc_info_t *procs_info, int procs_count, int curr_time,
         int curr_pid)
{

  int min_duration_time_to_finish = process_total_time(procs_info[0].pid) - procs_info[0].executed_time; // : tiempo minimo restante de ejecucion
  int next_procs_pid = procs_info[0].pid;
  for (int i = 0; i < procs_count; i++)
  {
    int process_time_to_finish = process_total_time(procs_info[i].pid) - procs_info[i].executed_time; // : tiempo minimo restante de ejecucion del proceso actual
    if (min_duration_time_to_finish > process_time_to_finish)
    {
      next_procs_pid = procs_info[i].pid;
      min_duration_time_to_finish = process_time_to_finish;
    }
  }

  return next_procs_pid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Round_Robin(proc_info_t *procs_info, int procs_count, int curr_time,
                int curr_pid)
{

  // actual_time_interrupt_count_to_time_slice: es la cantidad d time interrupt que un proceso se ha saltado mientras
  // se ejecutaba.Variable Global

  if (actual_time_interrupt_count_to_time_slice == Time_Slice / 10) // : si ya completaste tu slice time...
  {
    actual_time_interrupt_count_to_time_slice = 0;

    // round_robin_process_counter: es el index del proceso que se esta ejecutando en el array procs_info.Variable Global

    if (round_robin_process_counter < procs_count - 1) // : si no eres el ultimo ...
    {
      if (curr_pid != -1)
      {
        round_robin_process_counter++;
        return procs_info[round_robin_process_counter].pid; // : proceso q se va a ejecutar}
      }
      else
      {
        return procs_info[round_robin_process_counter].pid; // : proceso q se va a ejecutar
      }
    }
    else
    {
      round_robin_process_counter = 0;
      return procs_info[0].pid;
    }
  }

  if (curr_pid != -1) // : si todavia no has completado tu slice time y no has terminado de ejecutarte...
  {
    actual_time_interrupt_count_to_time_slice++;
    return curr_pid;
  }
  else // : si todavia no has completado tu slice time y ya terminaste de ejecutarte...
  {
    actual_time_interrupt_count_to_time_slice = 0;

    if (round_robin_process_counter < procs_count - 1) // : si no eres el ultimo...
    {
      return procs_info[round_robin_process_counter].pid;
    }
    else
    {
      round_robin_process_counter = 0;
      return procs_info[0].pid
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "SJF") == 0)
    return *SJF;
  if (strcmp(name, "STCF") == 0)
    return *STCF;
  if (strcmp(name, "Round_Robin") == 0)
    return *Round_Robin;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
