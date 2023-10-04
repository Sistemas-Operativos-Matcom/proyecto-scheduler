#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"
#include "simulation.h"
#define QUANTUM 20

Queue ready_queue;

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
  if (curr_pid == -1)
  {
    printf("\nCAMBIA\n");
    /* code */
  }
  else
  {
    // int exec_time = procs_info[0].executed_time;
    // printf("\n%i\n", process_total_time(procs_info[0].pid));
  }

  return procs_info[0].pid;
}
int my_SJF(proc_info_t *procs_info, int procs_count, int curr_time,
           int curr_pid)
{
  if (curr_pid == -1)
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
int my_STCF(proc_info_t *procs_info, int procs_count, int curr_time,
            int curr_pid)
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
int my_RR2(proc_info_t *procs_info, int procs_count, int curr_time,
           int curr_pid)
{
  if (curr_pid != -1)
  {
    int pos = findPID(procs_info, procs_count, curr_pid);
    if (procs_info[pos].executed_time % QUANTUM != 0)
      return curr_pid;
    else
    {
      actQueue(procs_info, procs_count, curr_pid);
      enqueue(&ready_queue, procs_info[pos]);
      proc_info_t next_proc = dequeue(&ready_queue);
      return next_proc.pid;
    }
  }
  else
  {
    actQueue(procs_info, procs_count, curr_pid);
    proc_info_t next_proc = dequeue(&ready_queue);
    return next_proc.pid;
  }
}
void actQueue(proc_info_t *procs_info, int procs_count,
              int curr_pid)
{
  for (int i = 0; i < procs_count; i++)
  {
    /* if (procs_info[i].pid == curr_pid)
    {
      continue;
    } */

    if (contains(&ready_queue, procs_info[i]) == 0)
    {
      enqueue(&ready_queue, procs_info[i]);
    }
  }
}
int my_RR(proc_info_t *procs_info, int procs_count, int curr_time,
          int curr_pid)
{
  /* annado a la cola los procesos nuevos que llegaron */
  int pos = -1;
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == curr_pid)
    {
      pos = i;
      continue;
    }

    if (contains(&ready_queue, procs_info[i]) == 0)
    {
      enqueue(&ready_queue, procs_info[i]);
    }
  }
  /* busco cual es el proceso del pid actual y verifico si ya cumplio su quantum */
  // int pos = findPID(procs_info, procs_count, curr_pid);
  if (curr_pid != -1 && pos != -1)
  {
    printf("si");
    if (procs_info[pos].executed_time % QUANTUM == 0)
    {
      printf("\nok:%i", curr_pid);
      enqueue(&ready_queue, procs_info[pos]);
    }
    else
    {
      return curr_pid;
    }
  }
  /* si la cola no esta vacia hago pop y retorno ese pid */
  if (!isEmpty(&ready_queue))
  {
    proc_info_t next_proc = dequeue(&ready_queue);
    printf("perfe: %i  cant cola:%i\n", next_proc.pid, ready_queue.pop);
    return next_proc.pid;
  }
  return -1;
}
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
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
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
    return *my_SJF;
  if (strcmp(name, "stcf") == 0)
    return *my_STCF;
  if (strcmp(name, "rr") == 0)
  {
    initQueue(&ready_queue);
    return *my_RR2;
  }

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
