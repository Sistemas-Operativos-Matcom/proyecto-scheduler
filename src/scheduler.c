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
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) 
{
  int PID;
  if (curr_pid == -1)
  {
    PID = procs_info[0].pid;
  }
  else
  {
    PID = curr_pid;
  }
  for (size_t i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == curr_pid)
    {
      return curr_pid;
    }
    if (process_total_time(procs_info[i].pid) < process_total_time(PID))
    {
      PID = procs_info[i].pid;
    }
  }
  return PID;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) 
{
  int PID = curr_pid;
  int time;
  if (curr_pid == -1)
  {
    time = process_total_time(procs_info[0].pid);
  }
  else
  {
    time = process_total_time(curr_pid);
  }
  for (size_t i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < time)
    {
      PID = procs_info[i].pid;
      time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    }
  }
  return PID;
}

int prr = 0;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  // aqui time slice = timer interrupt 
  // es eso bueno?
  if (curr_pid != -1)
  {
    prr ++;
  }
  if (prr == procs_count)
  {
    prr = 0;
  }
  return procs_info[prr].pid;
}

const int Times = 5;
int times = Times;

int rr5_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  //time slice = 50
  //que pasa cuando termina un proceso? 
  if (times != 0 && curr_pid != -1)
  {
    times --;
    return curr_pid;
  }
  times = Times;
  if (curr_pid != -1)
  {
    prr ++;
  }
  if (prr == procs_count)
  {
    prr = 0;
  }
  return procs_info[prr].pid;
}

Queue_t *createQueue_t(){
  Queue_t *queue = (Queue_t *)malloc(sizeof(Queue_t));
  queue->front = 0;
  queue->rear = 0;
  queue->data = (proc_info_t *)malloc(0 * sizeof(proc_info_t));
  return queue;
}

void enqueue(Queue_t *queue, proc_info_t process) {
  queue->data[queue->rear] = process;
  queue->rear++;
}

void dequeue(Queue_t *queue) 
{
  queue->front++;
}

proc_info_t get(Queue_t *queue)
{
  proc_info_t process = queue->data[queue->front];
  return process;
}

Queue_t empty ()
{
  Queue_t queue = {(proc_info_t *)0,0,0};
  return queue;
}

int lastprocess = -1;
int lastlastprocess = -1;
const int TimeSlice = 5;
int priority = 100;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{

  // if (curr_time % priority == 0)
  // {
    Queue_t *First = (Queue_t *)malloc(sizeof(Queue_t));
    First = createQueue_t(1000);
    Queue_t *Second = (Queue_t *)malloc(sizeof(Queue_t));
    Second = createQueue_t(1000);
    Queue_t *Third = (Queue_t *)malloc(sizeof(Queue_t));
    Third = createQueue_t(1000);
  // }
  
  for (int i = procs_count - 1; i > -1; i--)
  {
    if (procs_info[i].pid != lastprocess && procs_info[i].pid != lastlastprocess)
    {
      enqueue(First,procs_info[i]);
    }
  }

  lastprocess = procs_info[procs_count-1].pid;
  lastlastprocess = procs_info[procs_count-2].pid;

  if ((*First).front != (*First).rear)
  {
    proc_info_t proc = get(First);
    proc.executed_queue++;
    if (proc.executed_queue == TimeSlice)
    {
      dequeue(First);
      enqueue(Second, proc);
      proc.executed_queue = 0;
    }
    return proc.pid;
  }

  if ((*Second).front != (*Second).rear)
  {
    proc_info_t proc = get(Second);
    proc.executed_queue++;
    if (proc.executed_queue == TimeSlice)
    {
      dequeue(Second);
      enqueue(Third, proc);
      proc.executed_queue = 0;
    }
    return proc.pid;
  }

  if ((*Third).front != (*Third).rear)
  {
    proc_info_t proc = get(Third);
    proc.executed_queue++;
    if (proc.executed_queue == TimeSlice)
    {
      dequeue(Third);
      enqueue(Third, proc);
      proc.executed_queue = 0;
    }
    return proc.pid;
  }
  return -1;
}

/*int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}*/

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  //if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;
  
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
