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

// buscar otro proceso, aplicando rr, devuelve -1 si no se encontro. Retorna el indice del proceso en el array de Procs
int mlfq_find_other_process(int pid[], int level[], proc_info_t *procs, int procs_count, int depht_to_find, int depth_count, int *turn, int diferent_pid)
{
  if (depth_count == 0) // no hay procesos
    return -1;

  // Hacer la lista de procesos del nivel con mayor prioridad
  proc_info_t depth_procs[depth_count];
  mlfq_filter_procs_level(procs, level, procs_count, depht_to_find, depth_procs, diferent_pid); // llenar la lista

  // Hacer rr sobre los procesos
  int depth_procs_index = rr_manager(depth_procs, depth_count, 0, turn);
  int pid_proc = depth_procs[depth_procs_index].pid;
  int index = find_pid_array(procs, procs_count, pid_proc);

  return index;
}

int mlfq_manager(int pid[], int level[], int time[], proc_info_t *procs, int *pid_count, int procs_count, int MAX_DEPTH, int BOST_TIME, int TIME_SLICE, int TIME, int *turn, int current_time, int *original_pid, int current_pid)
{
  mlfq_merge(pid, level, time, procs, pid_count, procs_count); // actualizar los procesos

  if (current_time % BOST_TIME == 0) // priority boost
    mlfq_priority_bost(level, time, *pid_count);

  // buscar el nivel actual
  int depth_count = 0;
  int depth = mlfq_find_lowest_depth(procs, level, *pid_count, &depth_count, MAX_DEPTH, *original_pid);

  int index_orig = find_pid_array(procs, procs_count, *original_pid); // Buscar el proceso original
  printf("PID:%d INDEX:%d\n", *original_pid, index_orig);
  int flag_isSubstitute = 0;

  if (index_orig >= 0 && level[index_orig] <= depth) // si el proceso aun se encuentra y sigue siendo de prioridad
  {
    // printf("A");
    if (!procs[index_orig].on_io) // se esta ejecutando en cpu
    {
      mlfq_update_proc(pid, level, time, MAX_DEPTH, TIME_SLICE, TIME, index_orig);
      printf("!ON_IO:%d", index_orig);
      return index_orig;
    }
    // buscar un proceso substituto
    flag_isSubstitute = 1;
  }

  // Cambiar de proceso
  // buscar proceso un nuevo proceso para ejecutar
  int index_next_to_exec = mlfq_find_other_process(pid, level, procs, procs_count, depth, depth_count, turn, *original_pid);

  if (index_next_to_exec < 0) // No encontro otro proceso para ejecutar
    index_next_to_exec = find_pid_array(procs, procs_count, current_pid);

  if (!flag_isSubstitute) // seleccionar como el nuevo original
    *original_pid = procs[index_next_to_exec].pid;

  mlfq_update_proc(pid, level, time, MAX_DEPTH, TIME_SLICE, TIME, index_next_to_exec); // Actualizar sus stats

  return index_next_to_exec; // ejecutar
}

// Parametros del MLFQ
int mlfq_depth = 2; // cantidad de colas de prioridad o niveles del mlfq(indexado en 0)
int mlfq_priority_bost_time = 20 * TIME_INTERRUPT;
int mlfq_time_slice = 2 * TIME_INTERRUPT;

// Info sobre los procesos
int mlfq_pid[MAX_PROCESS_COUNT];       // array de los pid
int mlfq_level_pid[MAX_PROCESS_COUNT]; // nivel de cada proceso
int mlfq_time_pid[MAX_PROCESS_COUNT];  // tiempo de cada proceso en su nivel
int mlfq_count = 0;
int mlfq_turn = 0; // round robin turn

// Others
int pid_orig_proc = -1; // proceso que se estaba ejecutando originalmente y cedio el cpu

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int index = mlfq_manager(mlfq_pid, mlfq_level_pid, mlfq_time_pid, procs_info, &mlfq_count, procs_count, mlfq_depth, mlfq_priority_bost_time, mlfq_time_slice, TIME_INTERRUPT, &mlfq_turn, curr_time, &pid_orig_proc, curr_pid);
  printf("%d\n", index);
  return procs_info[index].pid;
}

// RANDOM
// Ejecuta un proceso random
int random_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  srand((unsigned)time(NULL));
  return procs_info[rand() % procs_count].pid;
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

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}