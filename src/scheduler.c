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

// SJF Shortest Job First
int compare_sjf(proc_info_t a, proc_info_t b) // Criterio de Comparacion
{
  int aTComplete = (a.on_io) ? __INT32_MAX__ : process_total_time(a.pid);
  int bTComplete = (b.on_io) ? __INT32_MAX__ : process_total_time(b.pid);
  return aTComplete - bTComplete;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int to_exec_index = select_item(procs_info, procs_count, compare_sjf);
  return procs_info[to_exec_index].pid;
}

// STCF Shortest Time to Completion First Schelduler
int compare_stcf(proc_info_t a, proc_info_t b) // Criterio de Comparacion
{
  int aTComplete = (a.on_io) ? __INT32_MAX__ : process_total_time(a.pid) - a.executed_time;
  int bTComplete = (b.on_io) ? __INT32_MAX__ : process_total_time(b.pid) - b.executed_time;
  return aTComplete - bTComplete;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int to_exec_index = select_item(procs_info, procs_count, compare_stcf);
  return procs_info[to_exec_index].pid;
}

// ROUND ROBIN
const int TIME_INTERRUPT = 10; // Time Interrump

int time_slice(int k) // parametrizar el rr
{
  k = (k == 0) ? 1 : k;
  return TIME_INTERRUPT * k;
}

int turn_procs = 0;

int pass_turn(int *turn)
{
  *turn += 1;
  return *turn - 1;
}

int rr_manager(proc_info_t *procs, int procs_count, int check_io, int *turn) // check_io != 0 si se quiere escoger un proceso q  no esta en io
{
  int next = 0;
  int current = pass_turn(turn) % procs_count;

  while (check_io && procs[current].on_io && next < procs_count)
  {
    current = pass_turn(turn) % procs_count;
    next++;
  }
  return current;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_time % time_slice(2) != 0)
    return curr_pid;

  int index = rr_manager(procs_info, procs_count, 0, &turn_procs) % procs_count;
  return procs_info[index].pid;
}

// MLFQ

int mlfq_manager(int pid[], int level[], int time[], proc_info_t *procs, int *pid_count, int procs_count, int MAX_DEPTH, int MAX_TIME, int BOST_TIME, int t_slice, int *turn, int current_pid, int current_time)
{
  // actualizar los procesos
  mlfq_merge(pid, level, time, procs, pid_count, procs_count); // a partir de aqui, Para todo i:pid[i] == procs.pid[i]

  // priority boost
  if (current_time % BOST_TIME == 0)
  {
    printf(" BOOST\n");
    mlfq_priority_bost(level, time, *pid_count);
  }

  // buscar el nivel actual
  int depth_count = 0;
  int depth = mlfq_find_lowest_depth(level, *pid_count, &depth_count, MAX_DEPTH);

  if (current_pid != -1) // verificar si el proceso esta en i/o durante el t.interrupt para priorizarlo
  {
    int current_proc_index = find_pid_array(procs, procs_count, current_pid);

    // si el proceso hizo io y ademas sigue siendo de prioridad entonces lo ejecuto
    if (procs[current_proc_index].on_io && level[current_proc_index] <= depth)
    {
      mlfq_update_proc(pid, level, time, MAX_DEPTH, MAX_TIME, t_slice, current_proc_index);
      return current_proc_index;
    }
  }

  // Hacer la lista de procesos del nivel con mayor prioridad
  proc_info_t depth_procs[depth_count];
  mlfq_filter_procs_level(level, procs, procs_count, depth, depth_procs); // llenar la lista

  // Hacer rr sobre los procesos
  int depth_procs_index = rr_manager(depth_procs, depth_count, 0, turn);

  // Actualizar sus stats
  int index = find_pid_array(procs, procs_count, depth_procs[depth_procs_index].pid);
  mlfq_update_proc(pid, level, time, MAX_DEPTH, MAX_TIME, t_slice, index);

  return index;
}

// Parametros del MLFQ
int mlfq_depth = 2; // cantidad de colas de prioridad o niveles del mlfq(indexado en 0)
int mlfq_priority_bost_time = 20 * TIME_INTERRUPT;
int mlfq_max_time_level = 4 * TIME_INTERRUPT;
int mlfq_time_slice = 2 * TIME_INTERRUPT;

// Info sobre los procesos
int mlfq_pid[MAX_PROCESS_COUNT];       // array de los pid
int mlfq_level_pid[MAX_PROCESS_COUNT]; // nivel de cada proceso
int mlfq_time_pid[MAX_PROCESS_COUNT];  // tiempo de cada proceso en su nivel
int mlfq_count = 0;
int mlfq_turn = 0; // round robin turn

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_time % mlfq_time_slice != 0 && curr_pid != -1)
    return curr_pid;

  int index = mlfq_manager(mlfq_pid, mlfq_level_pid, mlfq_time_pid, procs_info, &mlfq_count, procs_count, mlfq_depth, mlfq_max_time_level, mlfq_priority_bost_time, mlfq_time_slice, &mlfq_turn, curr_pid, curr_time);

  return procs_info[index].pid;
}

// SCHEDULER OTRAS IMPLEMENTACIONES:

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
// el proximo que siga en los procesos actuales es el que ejecuto

// para mejorar el tournaround, ademas voy guardando los procesos que son saltados por estan en io
// para ejecutarlos cuando dejen de estarlo

int rrplus_past_pid[MAX_PROCESS_COUNT]; // Array para guardar los procesos del ultimo time interrupt
int rrplus_was_io[MAX_PROCESS_COUNT];   // procesos que no se ejecutaron por estan io
int rrplus_past_pid_count = 0;

int rrplus_manager(int past_proc[], int was_io[], proc_info_t *current_procs, int *past_proc_count, int current_count, int *turn)
{
 
  // buscar si queda algun proceso pendiente
  int left = rr_find_lostProcess(past_proc, was_io, *past_proc_count, current_procs, current_count);
  if (left >= 0)
  {
    return left;
  }
  // No hay procesos pendientes
  int rr = find_match(past_proc, was_io, current_procs, past_proc_count, current_count, *turn);
  rr_merge(past_proc, was_io, current_procs, past_proc_count, current_count);
  *turn = rr;
  return rr;
}

int round_robin_plus_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_time % time_slice(2) != 0 || procs_count == 0)
    return curr_pid;

  int index = rrplus_manager(rrplus_past_pid, rrplus_was_io, procs_info, &rrplus_past_pid_count, procs_count, &turn_procs);
  return procs_info[index % procs_count].pid;
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

  if (strcmp(name, "mlfq") == 0)
    return *mlfq_scheduler;

  // Variaciones
  if (strcmp(name, "rand") == 0)
    return *random_scheduler;

  if (strcmp(name, "rr+") == 0)
    return *round_robin_plus_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}