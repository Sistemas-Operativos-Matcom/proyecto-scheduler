#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"
#include "simulation.h"
#include "queue_ll.h"
#define QUANTUM 50
#define TIC (QUANTUM / 10)
// La prioridad más alta
Queue Priority_0;
// La prioridad media
Queue Priority_1;
// La prioridad más baja
Queue Priority_2;
Queue RR;
// tNode *Pr1 = NULL;
// tNode *Pr2 = NULL;
// tNode *Pr3 = NULL;
// tNode *act = NULL;

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
// *name: sjf
int my_SJF(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid == -1 || findPID(procs_info, procs_count, curr_pid) == -1)
  {
    proc_info_t procmin = procs_info[0];
    int TTmin = process_total_time(procmin.pid);
    for (int i = 1; i < procs_count; i++)
    {
      int temp = process_total_time(procs_info[i].pid);
      if (temp < TTmin)
      {
        procmin = procs_info[i];
        TTmin = temp;
      }
    }
    return procmin.pid;
  }
  else
  {
    return curr_pid;
  }
}
// *name: stcf
int my_STCF(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  proc_info_t procmin = procs_info[0];
  int TTmin = process_total_time(procmin.pid);
  for (int i = 1; i < procs_count; i++)
  {
    int temp = process_total_time(procs_info[i].pid);
    if (temp < TTmin)
    {
      procmin = procs_info[i];
      TTmin = temp;
    }
  }
  return procmin.pid;
}
// *name: rr
int my_RR(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  static int pos = -1;
  int i = findPID(procs_info, procs_count, curr_pid);
  if (curr_pid == -1 || procs_info[i].executed_time % QUANTUM == 0 || i == -1)
  {
    //// print_proc_info(procs_info, procs_count);
    //// printf("pos: %d count: %d countentrpos: %d\n", pos, procs_count, pos % procs_count);
    return procs_info[++pos % procs_count].pid;
  }
  return curr_pid;
}
// *name: rrq   (q=> queue)
int my_RR_queue(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  //// printf("PID: %d timeex: %d totaltime: %d \n", curr_pid, procs_info[findPID(procs_info, procs_count, curr_pid)].executed_time, process_total_time(curr_pid));
  if (curr_pid != -1 && procs_info[findPID(procs_info, procs_count, curr_pid)].executed_time % QUANTUM != 0 && (findPID(procs_info, procs_count, curr_pid) != -1))
  {
    return curr_pid;
  }

  if (isEmpty(&RR))
  {
    for (size_t i = 0; i < procs_count; i++)
    {
      proc_info_t process = procs_info[i];
      enqueue(&RR, procs_info[i]);
    }
  }
  proc_info_t process = dequeue(&RR);
  while (findPID(procs_info, procs_count, process.pid) == -1)
  {
    if (isEmpty(&RR))
    {
      for (size_t i = 0; i < procs_count; i++)
        enqueue(&RR, procs_info[i]);
    }
    process = dequeue(&RR);
  }

  // printf("PID: %d timeex: %d totaltime: %d \n", process.pid, process.executed_time, process_total_time(process.pid));
  return process.pid;
  // }
}
void printQueue(Queue *q)
{
  int i = q->pop;

  if (isEmpty(q))
  {
    printf("Cola vacia\n");
  }
  else
  {
    printf("Cola: ");
    for (i = q->pop; i < q->puntero; i++)
    {
      printf("%d ", q->items[i].pid);
    }
    printf("\n");
  }
}
void print_proc_info(proc_info_t *procs_info, int procs_count)
{
  for (int i = 0; i < procs_count; i++)
  {
    printf("PID: %d ", procs_info[i].pid);
    printf("Priority: %d ", procs_info[i].priority);
    printf("Executed time: %d ", procs_info[i].executed_time);
    printf("On IO: %d ", procs_info[i].on_io);
    printf("Total time: %d\n", process_total_time(procs_info[i].pid));
    printf("\n");
  }
}
/*
int mi_MLFQ(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid != -1)
  {
    proc_info_t process_curr = procs_info[findPID(procs_info, procs_count, curr_pid)];
    if (process_curr.executed_time % QUANTUM != 0)
    {
      return curr_pid;
    }
    else
    {
      if (process_curr.on_io == 1)
      {
        enqueue(&Priority_0, process_curr);
      }
      else if (process_curr.priority == 0)
      {
        enqueue(&Priority_1, process_curr);
      }
      else if (process_curr.priority == 1)
      {
        enqueue(&Priority_2, process_curr);
      }
    }
  }

  for (int i = 0; i < procs_count; i++)
  {
    proc_info_t process = procs_info[i];
    if (!contains(&Priority_0, process) && !contains(&Priority_1, process) && !contains(&Priority_2, process) && process.pid != curr_pid)
    {
      enqueue(&Priority_0, process);
    }
  }
  if (!isEmpty(&Priority_0))
  {
    proc_info_t process = dequeue(&Priority_0);
    return process.pid;
  }
  else if (!isEmpty(&Priority_1))
  {
    proc_info_t process = dequeue(&Priority_1);
    return process.pid;
  }
  else if (!isEmpty(&Priority_2))
  {
    proc_info_t process = dequeue(&Priority_2);
    return process.pid;
  }
  else
    return -1;
}
int my_MLFQ(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  static int pos = -1;
  if (curr_time % QUANTUM == 0 || curr_pid == -1)
  {
    if (procs_info[++pos].priority == 0)
    {
      if (procs_info[pos].on_io == 0)
      {
        procs_info[pos].priority = 1;
      }

      return procs_info[pos % procs_count].pid;
    }
  }
  if (curr_time % QUANTUM == 0 || curr_pid == -1)
  {
    if (procs_info[++pos].priority == 1)
    {
      if (procs_info[pos].on_io == 1)
      {
        procs_info[pos].priority = 0;
      }

      return procs_info[pos % procs_count].pid;
    }
  }
  if (curr_time % QUANTUM == 0 || curr_pid == -1)
  {
    if (procs_info[++pos].priority == 2)
    {
      if (procs_info[pos].on_io == 1)
      {
        procs_info[pos].priority = 0;
      }
      return procs_info[pos % procs_count].pid;
    }
  }

  return curr_pid;
} */
int findPID(proc_info_t *procs_info, int procs_count, int pid)
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
int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;                 // PID del proceso
  int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
                                               // realizando una opreación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o e n I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:s
  int duration = process_total_time(pid);

  return -1;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  initQueue(&RR);

  // initQueue(&ready_queue);
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0)
    return *my_SJF;
  if (strcmp(name, "stcf") == 0)
    return *my_STCF;
  if (strcmp(name, "rr") == 0)
    return *my_RR;
  if (strcmp(name, "rrq") == 0)
    return *my_RR_queue;
  /* if (strcmp(name, "mlfq") == 0)
    return *my_MLFQ;
  if (strcmp(name, "mlfq2") == 0)
    return *mi_MLFQ; */
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
