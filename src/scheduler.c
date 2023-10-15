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


// Method to get proccesses with shortest duration time in procs_info list
proc_info_t getMin(proc_info_t *procs_info, int procs_count, int stcf)
{
  proc_info_t min = procs_info[0]; 
  int executed = 0;
  int minDuration = process_total_time(procs_info[0].pid);
  int i;
  for(i = 0; i < procs_count; i++)
  {
    // if scheduler is stcf, this method returns the process with shortest time to completion
    // else, it returns the shortest job process
    // The diferrence between one and another is exectudes time
    if(stcf == 1)
    {
      executed = procs_info[i].executed_time;
    }
    int duration = process_total_time((procs_info[i].pid)) - executed;
    if(duration < minDuration)
    {
      minDuration = duration;
      min = procs_info[i];
    }
  }
  
  return min;
}

typedef struct queue{
  int length;
  proc_info_t *processes_info;
  proc_info_t (*pop)(struct queue *q);
  void (*push)(proc_info_t process, struct queue *q);
} queue_t;

void push(proc_info_t process, struct queue *queue)
{
  queue->processes_info[queue->length-1] = process;
  queue->length+=1;
}

proc_info_t pop(struct queue *queue)
{
  proc_info_t proc = queue->processes_info[0];
  int i;
  for(i = 1; i < queue->length; i++)
  {
    queue->processes_info[i-1] = queue->processes_info[i];
  }
  queue->length-=1; 
  return proc;
}

queue_t queue_init(int max_procs)
{
  queue_t q0;
  q0.length = 0;
  q0.processes_info = (proc_info_t*)malloc(max_procs * sizeof(proc_info_t));
  q0.push = push;
  q0.pop = pop;
  return q0;
}

// queue_t queue = (queue_t *)malloc(sizeof(queue_t));

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

  if(curr_pid != -1)
  {
    return curr_pid;
  }
  return getMin(procs_info, procs_count, 0).pid;

}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return getMin(procs_info, procs_count, 1).pid;
}
int times = 5;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int i;
  for(i = 0; i < procs_count-1; i++)
  {
    if(curr_pid == procs_info[i].pid)
    {
      if(times == 0)
      {
        times = 5;
        return procs_info[i+1].pid;
      }
      else
      {
        times--;
        return curr_pid;
      }
    }
  }
  return procs_info[0].pid;
}


// Queues for MLFQ
int queuesQuant = 4;
queue_t * queues = (queue_t*) malloc(queuesQuant * sizeof(queue_t));

void init_queues(int procs_count)
{
  int i;
  for(i = 0; i < queuesQuant; i++)
  {
    queues[i] = queue_init(procs_count);
  }
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  init_queues(procs_count);

  // MLFQ

  
}

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  // int pid = procs_info[0].pid;      // PID del proceso
  // int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
  //                                   // realizando una opreación IO
  // int exec_time =
  //     procs_info[0].executed_time;  // Tiempo que lleva el proceso activo
  //                                   // (curr_time - arrival_time)

  // // También puedes usar funciones definidas en `simulation.h` para extraer
  // // información extra:
  // int duration = process_total_time(pid);

  return -1;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;



  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
