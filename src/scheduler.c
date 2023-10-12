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

const int MAX_PROCS = 205;

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

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

int pass_turn(int *turn)
{
  turn += 1;
  return turn - 1;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return (curr_time % rr_time_slice(TIME_INTERRUPT) == 0) ? procs_info[pass_turn(&turn_procs) % procs_count].pid : curr_pid;
}

// MLFQ

// Parametros del MLFQ
int mlfq_depth = 3; // cantidad de colas de prioridad o niveles del mlfq
int mlfq_priority_boot_time = 10;
int mlfq_max_time_level = 10;

// Info sobre los procesos
int mlfq_past_pid[MAX_PROCS];  // array de los pid
int mlfq_level_pid[MAX_PROCS]; // nivel de cada proceso
int mlfq_time_pid[MAX_PROCS];  // tiempo de cada proceso en su nivel
int count = 0;

// Actualizar los procesos, buscar nuevos
void merge_update(int past_pid[], int level_pid[], int time_pid[], proc_info_t *current_procs, int *past_count, int procs_count)
{
  int ipp = 0;   // index of pid
  int icp = 0;   // index of current pid
  int icopy = 0; // index of copy to past_pid

  // buscar los procesos que aun se mantienen activos
  // como los procesos estan en orden de llegada, a partir de la posicion de un proceso nuevo hacia atras todos seran nuevos
  while (ipp < past_count && icp < procs_count)
  {
    if (past_pid[ipp] == current_procs[icp].pid)
    {
      past_pid[icopy] = past_pid[ipp];
      level_pid[icopy] = past_pid[ipp];
      time_pid[icopy] = past_pid[ipp];
      icp++;
      icopy++;
    }
    ipp++;
  }
  // annadir  los nuevos
  while (icopy < procs_count)
  {
    past_pid[icopy] = current_procs[icp].pid;
    level_pid[icopy] = 0;
    time_pid[icopy] = 0;
  }
  *past_count = procs_count;
  return;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return curr_pid;
}

// SCHEDULER OTRAS IMPLEMENTACIONES

// RANDOM
// Ejecuta un proceso random
int random_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  srand((unsigned)time(NULL));
  return procs_info[rand() % procs_count].pid;
}

// ROUND ROBIN PLUS
// En el rr puede pasar el proceso ejecutado termine durante el time slice, entonces
// el proceso que venia ocupa el index del que termino, y es saltado.
// De forma general si varios procesos terminan durante el time slice
// van a alterar el orden de los turnos del rr

// para evitar esto voy a buscar de los procesos que me quedaron en el interrupt anterior
// el proximo a partir de donde me quede, que siga en los procesos actuales.

// Guardar los procesos del ultimo t.interrupt
void save_procs(int dest[], proc_info_t *source, int *dest_count, int source_count)
{
  *dest_count = source_count;
  for (int i = 0; i < source_count; i++)
    dest[i] = source[i].pid;
}

// buscar de los procesos que me faltaban por ejecutar, el primero que aun este en los procesos actuales
int find_match(int past_pid[], proc_info_t *current_procs, int *past_count, int current_count, int turn)
{
  int next_turn = 0;
  int last_index = (turn) % (*past_count);
  for (int i = last_index; i < *past_count; i++)
  {
    int temp = find_pid_array(current_procs, current_count, past_pid[i]);
    if (temp > 0)
    {
      next_turn = temp;
      break;
    }
  }
  save_procs(past_pid, current_procs, past_count, current_count);
  return next_turn;
}

int next_proc(int past_proc[], proc_info_t *current_procs, int past_proc_count, int count, int *turn)
{
  int next = find_match(past_proc, current_procs, past_proc_count, count, *turn);
  turn = next + 1;
  return next;
}

int rr_past_pid[MAX_PROCS]; // Array para guardar los procesos del ultimo time interrupt
int rr_past_pid_count = 0;

int round_robin_plus_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return (curr_time % rr_time_slice(TIME_INTERRUPT) == 0) ? procs_info[next_proc(rr_past_pid, procs_info, rr_past_pid_count, procs_count, &turn_procs) % procs_count].pid : curr_pid;
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

  if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;

  if (strcmp(name, "rr") == 0)
    return *round_robin_scheduler;

  // Variaciones
  if (strcmp(name, "rand") == 0)
    return *random_scheduler;

  if (strcmp(name, "rr+") == 0)
    return *round_robin_plus_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}