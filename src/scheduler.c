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

// Metodos Auxiliares
// Retorna el index de un processo en el array de procs_info o -1 si no se encuentra
int find_pid_array(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return i;
    }
  }
  return -1;
}

int find_pid_int(int pid_procs[], int procs_count, int pid)
{
  for (int i = 0; i < procs_count; i++)
  {
    if (pid_procs[i] == pid)
    {
      return i;
    }
  }
  return -1;
}

// Recorre y retorna el proceso segun comparacion
int select_item(proc_info_t *procs, int count, int comparer(proc_info_t, proc_info_t))
{
  int selected = 0;
  for (int i = 0; i < count; i++)
  {
    if (comparer(procs[i], procs[selected]) < 0)
    {
      selected = i;
    }
  }
  return selected;
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
int rr_time_slice(int timeInterrup) // parametrizar el rr
{
  int mult = 2;
  return timeInterrup * mult;
}

int turn_procs = 0;
const int TIME_INTERRUPT = 10; // Time Interrump

int pass_turn(int *turn)
{
  *turn += 1;
  return *turn - 1;
}

int get_rr_index(proc_info_t *procs, int procs_count, int check_io) // check_io != 0 si se quiere escoger un proceso q  no esta en io
{
  int next = 0;
  int current = pass_turn(&turn_procs) % procs_count;

  while (check_io && procs[current].on_io && next < procs_count)
  {
    current = pass_turn(&turn_procs) % procs_count;
    next++;
  }
  return current;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return (curr_time % rr_time_slice(TIME_INTERRUPT) == 0) ? procs_info[get_rr_index(procs_info, procs_count, 0) % procs_count].pid : curr_pid;
}

// MLFQ

// Metodos Utiles
// Actualizar los procesos, buscar nuevos
void mlfq_merge(int past_pid[], int level_pid[], int time_pid[], proc_info_t *current_procs, int *past_count, int procs_count)
{
  int ipp = 0;   // index of pid
  int icp = 0;   // index of current pid
  int icopy = 0; // index of copy to past_pid

  // buscar los procesos que aun se mantienen activos
  // como los procesos estan en orden de llegada, a partir de la posicion que este proceso nuevo hacia atras todos
  // seran nuevos en mi t.interrp
  while (ipp < (*past_count) && icp < procs_count)
  {
    if (past_pid[ipp] == current_procs[icp].pid) // actualizar los stats del proceso
    {
      past_pid[icopy] = past_pid[ipp];
      level_pid[icopy] = level_pid[ipp];
      time_pid[icopy] = time_pid[ipp];
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
    icopy++;
    icp++;
  }
  *past_count = procs_count; // actualizar el count valido
  return;
}

void priority_bost(int levels[], int time_level[], int count)
{
  for (int i = 0; i < count; i++)
  {
    if (levels[i] != 0) // Para no cambiar el time level, de los que ya estaban en el lv 0
    {
      levels[i] = 0;
      time_level[i] = 0;
    }
  }
  return;
}

// update the level of a process and returns 1 if the process has consumed all the time of the level
int update_select_proc(int pid[], int level[], int time_level[], int MAX_LEVEL, int MAX_TIME_LEVEL, int TIME_SLICE, int count, int pid_selected)
{
  int index = find_pid_int(pid, count, pid_selected);
  time_level[index] += TIME_SLICE;

  if (time_level[index] >= MAX_TIME_LEVEL)
  {
    level[index] = (level[index] + 1 >= MAX_LEVEL) ? MAX_LEVEL : level[index] + 1;
    time_level[index] = 0;
    return 1;
  }
  return 0;
}

// Parametros del MLFQ
int mlfq_depth = 2; // cantidad de colas de prioridad o niveles del mlfq(indexado en 0)
int mlfq_priority_bost_time = 10;
int mlfq_max_time_level = 10;

// Info sobre los procesos
int mlfq_past_pid[MAX_PROCESS_COUNT];  // array de los pid
int mlfq_level_pid[MAX_PROCESS_COUNT]; // nivel de cada proceso
int mlfq_time_pid[MAX_PROCESS_COUNT];  // tiempo de cada proceso en su nivel
int count = 0;

int mlfq_manager()
{
  return 0;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return curr_pid;
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

void rr_merge(int past_pid[], int was_io[], proc_info_t *current_procs, int *past_count, int procs_count)
{
  int ipp = 0;
  int icp = 0;
  int icopy = 0;

  while (ipp < (*past_count) && icp < procs_count)
  {
    if (past_pid[ipp] == current_procs[icp].pid) // actualizar los stats del proceso
    {
      past_pid[icopy] = past_pid[ipp];
      was_io[icopy] = was_io[icopy];
      icp++;
      icopy++;
    }
    ipp++;
  }
  // annadir  los nuevos
  while (icopy < procs_count)
  {
    past_pid[icopy] = current_procs[icp].pid;
    was_io[icopy] = 0;
    icopy++;
    icp++;
  }
  *past_count = procs_count; // actualizar el count valido
  return;
}

// buscar de los procesos que me faltaban por ejecutar, el primero que aun este en los procesos actuales
// para evitar el corrimiento de los turnos
int find_match(int past_pid[], int was_io[], proc_info_t *current_procs, int *past_count, int current_count, int turn)
{
  int next_proc = 0;

  if (*past_count)
  {
    int last_index = (turn) % (*past_count);
    for (int i = last_index + 1; i < *past_count; i++)
    {
      int temp = find_pid_array(current_procs, i + 1, past_pid[i]);
      if (temp >= 0)
      {
        if (current_procs[temp].on_io)
          was_io[i] = 1;
        else
        {
          next_proc = temp;
          break;
        }
      }
    }
  }
  return next_proc;
}

// buscar el primer proceso que no se ejecuto por estar en i/o
int rr_find_lostProcess(int pid[], int was_io[], int count, proc_info_t *currents_procs, int current_count)
{
  for (int i = 0; i < count; i++)
  {
    int temp = find_pid_array(currents_procs, current_count, pid[i]);

    if (was_io[i] && temp >= 0 && !currents_procs[temp].on_io)
    {
      was_io[i] = 0;
      return temp;
    }
  }
  return -1;
}

int rrplus_past_pid[MAX_PROCESS_COUNT]; // Array para guardar los procesos del ultimo time interrupt
int rrplus_was_io[MAX_PROCESS_COUNT];   // procesos que no se ejecutaron por estan io
int rrplus_past_pid_count = 0;

int next_proc(int past_proc[], int was_io[], proc_info_t *current_procs, int *past_proc_count, int current_count, int *turn)
{
  // priorizar la ejecucion de procesos que no se ejecutaron x estan en io por encima de los turnos de rr,
  // puede aumentar el response time, pero disminuira el turnaround time

  // buscar primero si queda algun proceso pendiente
  int left = rr_find_lostProcess(past_proc, was_io, *past_proc_count, current_procs, current_count);
  if (left >= 0)
  {
    return left;
  }
  // si no hay procesos pendientes ejecuto rr normal
  int rr = find_match(past_proc, was_io, current_procs, past_proc_count, current_count, *turn);
  rr_merge(past_proc, was_io, current_procs, past_proc_count, current_count);
  *turn = rr;
  return rr;
}

int round_robin_plus_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_time % rr_time_slice(TIME_INTERRUPT) != 0 || procs_count == 0)
    return curr_pid;

  int index = next_proc(rrplus_past_pid, rrplus_was_io, procs_info, &rrplus_past_pid_count, procs_count, &turn_procs);
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

  // Variaciones
  if (strcmp(name, "rand") == 0)
    return *random_scheduler;

  if (strcmp(name, "rr+") == 0)
    return *round_robin_plus_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}