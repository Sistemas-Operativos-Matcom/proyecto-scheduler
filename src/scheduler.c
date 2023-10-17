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

// Returns the index of a process with a given pid, if not found returns -1
int get_index(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int c = 0; c < procs_count; c++)
  {
    if (procs_info[c].pid == pid)
    {
      return c;
    }
  }

  return -1;
}

int calculate_procs_not_on_io(proc_info_t *procs_info, int procs_count)
{
  int procs_not_on_io_count = 0;
  for (int c = 0; c < procs_count; c++)
  {
    if (!procs_info[c].on_io)
    {
      procs_not_on_io_count++;
    }
  }

  return procs_not_on_io_count;
}

// Eliminates the processes on IO from procs_info
void remove_io_procs(proc_info_t *procs_info, proc_info_t *procs_not_on_io_info, int procs_count, int procs_not_on_io_count)
{
  int proc_index = 0;
  for (int c = 0; c < procs_count; c++)
  {
    if (!procs_info[c].on_io)
    {
      procs_not_on_io_info[proc_index++] = procs_info[c];
    }
  }
}

// Returns the pid of the next process that is not on IO
int next_not_on_io_proc(proc_info_t *procs_info, int procs_count, int index)
{
  for (int c = index; c < procs_count; c++)
  {
    if (!procs_info[c].on_io)
    {
      return procs_info[c].pid;
    }
  }

  for (int c = 0; c < index; c++)
  {
    if (!procs_info[c].on_io)
    {
      return procs_info[c].pid;
    }
  }

  return -1;
}

// First In First Out
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int procs_not_on_io_count = calculate_procs_not_on_io(procs_info, procs_count);
  if (procs_not_on_io_count == 0)
    return -1;
  proc_info_t procs_not_on_io_info[procs_not_on_io_count];
  remove_io_procs(procs_info, procs_not_on_io_info, procs_count, procs_not_on_io_count);

  // if(procs_info[0].on_io && procs_count>1)
  //   return procs_info[1].pid;

  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).

  return procs_not_on_io_info[0].pid;
  // return next_not_on_io_proc(procs_info, procs_count, 0);
}

// Shortest Job First
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int procs_not_on_io_count = calculate_procs_not_on_io(procs_info, procs_count);
  proc_info_t procs_not_on_io_info[procs_not_on_io_count];
  if (procs_not_on_io_count == 0)
    return -1;
  remove_io_procs(procs_info, procs_not_on_io_info, procs_count, procs_not_on_io_count);

  int min_duration = process_total_time(procs_not_on_io_info[0].pid);
  int selected_proc = procs_not_on_io_info[0].pid;

  for (int c = 1; c < procs_not_on_io_count; c++)
  {
    int actual_proc_duration = process_total_time(procs_not_on_io_info[c].pid);
    if (actual_proc_duration < min_duration)
    {
      min_duration = actual_proc_duration;
      selected_proc = procs_not_on_io_info[c].pid;
    }
  }

  return selected_proc;
}

// Shortest Time to Completion First
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int procs_not_on_io_count = calculate_procs_not_on_io(procs_info, procs_count);
  proc_info_t procs_not_on_io_info[procs_not_on_io_count];
  if (procs_not_on_io_count == 0)
    return -1;
  remove_io_procs(procs_info, procs_not_on_io_info, procs_count, procs_not_on_io_count);

  int min_time_left = process_total_time(procs_not_on_io_info[0].pid) - procs_not_on_io_info[0].executed_time;
  int selected_proc = procs_not_on_io_info[0].pid;

  for (int c = 1; c < procs_not_on_io_count; c++)
  {
    int actual_proc_time_left = process_total_time(procs_not_on_io_info[c].pid) - procs_not_on_io_info[c].executed_time;
    if (actual_proc_time_left < min_time_left)
    {
      min_time_left = actual_proc_time_left;
      selected_proc = procs_not_on_io_info[c].pid;
    }
  }

  return selected_proc;
}

int slice_time = 5 * 10; // !!!!!!INVESTIGATE HOW TO ACCESS TIMER INTERRUPT
int rrindex = 0;
// Round Robin
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int procs_not_on_io_count = calculate_procs_not_on_io(procs_info, procs_count);
  proc_info_t procs_not_on_io_info[procs_not_on_io_count];
  if (procs_not_on_io_count == 0)
    return -1;
  remove_io_procs(procs_info, procs_not_on_io_info, procs_count, procs_not_on_io_count);

  // int curr_index = get_index(procs_info, procs_count, curr_pid);
  // int selected_proc = curr_index == -1 ? procs_info[0].pid : procs_info[curr_index].pid;

  if (curr_time % slice_time == 0)
  {
    rrindex++;
    rrindex %= procs_not_on_io_count;
    /*
    int rrindex = 0;
    for (int c = 0; c < procs_count; c++)
    {
      rrindex = c;
      if (curr_pid == procs_info[c].pid)
        break;
    }
    rrindex++;
    rrindex %= procs_count;
    selected_proc = procs_info[rrindex].pid;
    //printf("sc%d\n",selected_proc);
    */
  }

  return procs_not_on_io_info[rrindex].pid;
}

int priority[100000];
int cpu_time[100000];
int max_cpu_time_allowed = 5 * 10;
int boost_time = 10 * 10;
// Multi-level Feedback Queue
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // for(int c=0; c<procs_count; c++)
  //{
  //   printf(" %d,", procs_info[c].pid);
  // }
  // printf("\n");

  for (int c = 0; c < procs_count; c++)
  {
    int proc_pid = procs_info[c].pid;
    if (!procs_info[c].on_io)
    {
      cpu_time[proc_pid] += 10; // a.k.a += timer_interrupt;
    }
    if (cpu_time[proc_pid] >= max_cpu_time_allowed)
    {
      priority[proc_pid]++;
      cpu_time[proc_pid] = 0;
    }
  }

  if (curr_time % boost_time == 0)
  {
    for (int c = 0; c < procs_count; c++)
    {
      int proc_pid = procs_info[c].pid;
      priority[proc_pid] = 0;
      cpu_time[proc_pid] = 0;
    }
  }

  int procs_not_on_io_count = calculate_procs_not_on_io(procs_info, procs_count);
  proc_info_t procs_not_on_io_info[procs_not_on_io_count];
  if (procs_not_on_io_count == 0)
    return -1;
  remove_io_procs(procs_info, procs_not_on_io_info, procs_count, procs_not_on_io_count);

  int min_priority = 1e9;
  int same_priority_amount = 0;
  for (int c = 0; c < procs_not_on_io_count; c++)
  {
    int proc_pid = procs_not_on_io_info[c].pid;
    if (priority[proc_pid] < min_priority)
    {
      min_priority = priority[proc_pid];
      same_priority_amount = 0;
    }
    if (priority[proc_pid] == min_priority)
    {
      same_priority_amount++;
    }
  }

  proc_info_t high_priority_procs[same_priority_amount];
  int hppindex = 0;
  for (int c = 0; c < procs_not_on_io_count; c++)
  {
    int proc_pid = procs_not_on_io_info[c].pid;
    if (priority[proc_pid] == min_priority)
    {
      high_priority_procs[hppindex++] = procs_not_on_io_info[c];
    }
  }

  // for(int c=0; c<same_priority_amount; c++)
  //{
  //   printf(" %d,", high_priority_procs[c].pid);
  // }

  return rr_scheduler(high_priority_procs, same_priority_amount, curr_time, curr_pid);
  }

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

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

/*
int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;                 // PID del proceso
  int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
                                               // realizando una operación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}
*/