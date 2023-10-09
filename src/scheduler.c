#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

#include "auxiliar.h"

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

// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
// {
//   // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
//   // cambiar)

//   // Información que puedes obtener de un proceso
//   int pid = procs_info[0].pid;                 // PID del proceso
//   int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
//                                                // realizando una operación IO
//   int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
//                                                // ejecutado (en CPU o en I/O)

//   // También puedes usar funciones definidas en `simulation.h` para extraer
//   // información extra:
//   int duration = process_total_time(pid);

//   return -1;
// }

// SJF Shortest Job First
int compare_sjf(const void *a, const void *b) // Criterio de Comparacion
{
  proc_info_t *x = (proc_info_t *)a;
  proc_info_t *y = (proc_info_t *)b;
  int xTComplete = (x->on_io) ? __INT32_MAX__ : process_total_time(x->pid);
  int yTComplete = (x->on_io) ? __INT32_MAX__ : process_total_time(y->pid);
  return xTComplete - yTComplete;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int to_exec_index = select_item(procs_info, procs_count, sizeof(proc_info_t), compare_sjf);
  return procs_info[to_exec_index].pid;
}

// STCF Shortest Time to Completion First Schelduler
int compare_stcf(const void *a, const void *b) // Criterio de Comparacion
{
  proc_info_t *x = (proc_info_t *)a;
  proc_info_t *y = (proc_info_t *)b;

  int xTComplete = (x->on_io) ? __INT32_MAX__ : process_total_time(x->pid) - x->executed_time;
  int yTComplete = (x->on_io) ? __INT32_MAX__ : process_total_time(y->pid) - y->executed_time;
  return xTComplete - yTComplete;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int to_exec_index = select_item(procs_info, procs_count, sizeof(proc_info_t), compare_stcf);
  return procs_info[to_exec_index].pid;
}

// ROUND ROBIN
int rr_time_slice(int timeInterrup) // parametrizar el rr
{
  int mult = 2;
  return timeInterrup * mult;
}

int turn_procs = 0;
const int TIME_INTERRUPT = 10; // Time Interrump

int pass_turn()
{
  turn_procs += 1;
  return turn_procs - 1;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return (curr_time % rr_time_slice(TIME_INTERRUPT) == 0) ? procs_info[pass_turn() % procs_count].pid : curr_pid;
}

//MLFQ


// SCHEDULER OTRAS IMPLEMENTACIONES

// RANDOM
int random_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  srand((unsigned)time(NULL));
  return procs_info[rand() % procs_count].pid;
}

// ROUND ROBIN PLUS
// En el rr puede pasar el proceso ejecutado termine durante el time slice, entonces
// el proceso que venia ocupa el index del que termino, y es saltado.
// De forma general si algun proceso se encontra en i/o y termina durante el time slice,
// va alterar el orden de los turnos del rr

proc_info_t past_procs_info[205]; // Array para guardar los procesos del ultimo time interrupt
int past_procs_info_count = 0;

void save_procs(proc_info_t dest[], proc_info_t *source, int *dest_count, int source_count)
{
  *dest_count = source_count;
  for (int i = 0; i < source_count; i++)
    dest[i] = source[i];
}

int find_match(proc_info_t past_procs[], proc_info_t *current_procs, int past_count, int current_count)
{
  int next_turn = 0;
  int last_index = (turn_procs - 1) % past_count;

  if (past_count == 0 || (last_index < current_count && current_procs[last_index].pid == past_procs[last_index].pid))
  {
    next_turn = turn_procs;
  }

  else // el orden fue alterado
  {
    for (int i = last_index; i < past_count; i++) // buscar el siguiente proceso
    {
      int temp = find_pid_array(current_procs, current_count, past_procs[i].pid);

      if (temp >= 0)
      {
        next_turn = temp;
        break;
      }
    }
  }

  save_procs(past_procs, current_procs, &past_procs_info_count, current_count);

  return next_turn;
}

int next_proc(proc_info_t *current_procs, int count)
{
  int next = find_match(past_procs_info, current_procs, past_procs_info_count, count);
  turn_procs = next + 1;
  return next;
}

int round_robin_plus_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return (curr_time % rr_time_slice(TIME_INTERRUPT) == 0) ? procs_info[next_proc(procs_info, procs_count) % procs_count].pid : curr_pid;
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
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //
  if (strcmp(name, "rand") == 0)
    return *random_scheduler;

  if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;

  if (strcmp(name, "rr") == 0)
    return *round_robin_scheduler;

  // Variaciones
  if (strcmp(name, "rr+") == 0)
    return *round_robin_plus_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}