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
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{

  // Se devuelve el PID del proceso que tenga menor tiempo de ejecución en total.
  // Si no se ha terminado de ejecutar el proceso actual, se continúa con ese.
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
  // Se devuelve el PID del proceso al que menos tiempo de ejecución le queda.
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

int slice_time = 0; // No es en realidad el slice time, sino el contador suma 1 en cada time interrupt hasta alcanzar el slice time.
int proc_index = 0; // Índice que indica a qué proceso le toca.
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Se cambia de proceso si se cumplió el slice time del proceso actual o si el proceso terminó su ejecución.
  if (curr_pid != -1)
  {
    if (slice_time < 3) // Se tomó un slice time de 30ms (3*time interrupt)
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

// se simula el funcionamiento de las colas de prioridad con los siguientes 3 arrays
int pids[MAX_PROCESS_COUNT];         // array de PIDs
int priorities[MAX_PROCESS_COUNT];   // array de Prioridades
int timeInQueues[MAX_PROCESS_COUNT]; // array de tiempos de los procesos en su cola actual
int priorityBoostTime = 0;           // mismo principio que el slice time, contador del boost time
int lastProcessIndex = 0;            // Índice que representa el lugar del array hasta el cual están los procesos actuales, es para simular que el array es dinámico.
int queuesCount = 3;                 // cantidad de colas

// Téngase en cuenta que se consideró la prioridad 0 como la máxima y 2 como la mínima.

void update(proc_info_t *procs_info, int procs_count)
{

  // Función para actualizar las "colas de prioridad". Contempla que se comleten procesos y lleguen nuevos.
  // Se tiene en cuenta que los procesos se agregan al array en el mismo orden en que van llegando
  int pid_index = 0;
  for (int i = 0; i < procs_count; i++)
  {
    int flag = 1;
    for (int j = 0; j < lastProcessIndex; j++)
    {
      if (procs_info[i].pid == pids[j])
      {
        pids[pid_index] = pids[j];
        priorities[pid_index] = priorities[j];
        timeInQueues[pid_index] = timeInQueues[j] + 1;
        pid_index++;
        flag = 0;
        break;
      }
    }
    if (flag)
    // si no se encontró un proceso de procs_info en las "colas de prioridad", se agregan todos a partir de ese con prioridad máxima.
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
  // lleva todos los procesos a prioridad máxima.
  for (int i = 0; i <= lastProcessIndex; i++)
  {
    priorities[i] = 0;
    timeInQueues[i] = 0;
  }
}

int turn = 0;
int mlfq_rr(int count)
// pequeño rr para el mlfq
{
  turn++;
  return (turn - 1) % count;
}

int FindNextProcess(proc_info_t *procs_info, int curr_time, int curr_pid)
{

  // Aquí es donde se devuelve el proceso que, según las reglas de prioridad, debe ser ejecutado
  int countOfQueue = 0;
  int currQueue = 0;
  for (int k = 0; k < queuesCount; k++)
  // Se cuentan los procesos con prioridad k
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
  // se arma el array para el rr
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
    int index = mlfq_rr(countOfQueue); // rr
    pidForReturn = procsInQueue[index].pid;
  }

  for (int i = 0; i < lastProcessIndex; i++)
  {
    if (pids[i] == pidForReturn)
    {
      timeInQueues[i]++;
      if (timeInQueues[i] == 3 && priorities[i] < queuesCount - 1)
      // bajar de prioridad un proceso cuando cumple un tiempo igual al slice time en una cola
      {
        priorities[i]++;
        timeInQueues[i] = 0;
      }
    }
  }

  return pidForReturn;
}
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Multi Level Feedback Queue
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
