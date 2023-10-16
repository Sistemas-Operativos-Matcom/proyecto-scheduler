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

// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid)
// {
//   // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
//   // cambiar)

//   // Información que puedes obtener de un proceso
//   int pid = procs_info[0].pid;                 // PID del proceso
//   int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
//                                                // realizando una opreación IO
//   int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
//                                                // ejecutado (en CPU o en I/O)

//   // También puedes usar funciones definidas en `simulation.h` para extraer
//   // información extra:
//   int duration = process_total_time(pid);

//   return -1;
// }
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid != -1)
    return curr_pid;
  int min = __INT_MAX__;
  int pid = procs_info[0].pid;
  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) < min)
    {
      min = process_total_time(procs_info[i].pid);
      pid = procs_info[i].pid;
    }
  }
  return pid;
}
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int min = __INT_MAX__;
  int pid = procs_info[0].pid;
  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < min)
    {
      min = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      pid = procs_info[i].pid;
    }
  }
  return pid;
}

int slice_time = 0;
int proc_index = 0;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid != -1)
  {
    if (slice_time < 3)
    {
      slice_time++;
      return curr_pid;
    }
    else
    {
      slice_time = 0;
      return procs_info[proc_index = ((proc_index + 1) % procs_count)].pid;
    }
  }
  else
  {
    slice_time = 0;
    return procs_info[proc_index = (proc_index % procs_count)].pid;
  }
}
int pids[MAX_PROCESS_COUNT];
int priorities[MAX_PROCESS_COUNT];
int timeInQueues[MAX_PROCESS_COUNT];
int priorityBoostTime = 0;
int lastProcessIndex = 0;
int queuesCount = 3;

void update(proc_info_t *procs_info, int procs_count)
{
  int pid_index = 0;
  int flag = 1;
  for (int i = 0; i < procs_count; i++)
  {
    for (int j = 0; j < lastProcessIndex; j++)
    {
      if (procs_info[i].pid == pids[j])
      {
        pids[pid_index] = pids[j];
        priorities[pid_index] = priorities[j];
        timeInQueues[pid_index] = timeInQueues[j] + 1;
        pid_index++;
        flag = 0;
      }
    }
    if (flag)
    {
      for (int k = i; k < procs_count; k++)
      {
        pids[pid_index] = procs_info[k].pid;
        priorities[pid_index] = 0;
        timeInQueues[pid_index] = 0;
        pid_index++;
      }
      break;
    }
  }
  lastProcessIndex = procs_count;
}

void priority_boost()
{
  for (int i = 0; i <= lastProcessIndex; i++)
  {
    priorities[i] = 0;
    timeInQueues[i] = 0;
  }
}

int turn = 0;
int mlfq_rr(int count)
{
  turn++;
  return (turn - 1) % count;
}

int FindNextProcess(proc_info_t *procs_info, int curr_time, int curr_pid)
{
  int countOfQueue = 0;
  int currQueue = 0;
  for (int k = 0; k < queuesCount; k++)
  {
    for (int i = 0; i < lastProcessIndex; i++)
    {
      if (priorities[i] == k && !procs_info[i].on_io)
      {
        countOfQueue++;
      }
    }

    if (countOfQueue > 0)
    {
      currQueue = k;
      break;
    }
  }

  proc_info_t procsInQueue[countOfQueue];

  int indexForRR = 0;

  for (int t = 0; t < lastProcessIndex; t++)
  {
    if (priorities[t] == currQueue && !procs_info[t].on_io)
    {
      procsInQueue[indexForRR] = procs_info[t];
      indexForRR++;
    }
  }

  int pidForReturn = procs_info[0].pid;

  if (countOfQueue != 0)
  {
    int index = mlfq_rr(countOfQueue);
    pidForReturn = procsInQueue[index].pid;
  }

  for (int i = 0; i < lastProcessIndex; i++)
  {
    if (pids[i] == pidForReturn)
    {
      timeInQueues[i]++;
      if (timeInQueues[i] == 3 && priorities[i] < queuesCount - 1)
      {
        priorities[i]++;
      }
    }
  }

  return pidForReturn;
}
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  update(procs_info, procs_count);

  if (priorityBoostTime == 6)
  {
    priority_boost();
    priorityBoostTime = 0;
  }

  else
  {
    priorityBoostTime++;
  }
  return FindNextProcess(procs_info, curr_time, curr_pid);
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
    return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0)
    return *mlfq_scheduler;
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
