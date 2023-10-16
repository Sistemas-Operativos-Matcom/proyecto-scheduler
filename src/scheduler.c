#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"
const QUEUES_AMOUNT = 5;
int curr_proc_pos = 0;
int time_slice = -10;
int time_slice_for_rr = -10;
int last_pid_in_queue = -1;
int queue_initialized = 0;
int current_queue = 0;
int priority_boost_count = -10;
int priority_boost = 400;
Queue** queues;

printQueue(int queueID) {
  ProcessNode* p = queues[queueID]->first;
    while (p != NULL) 
    {
      printf("%d , ",p->process->pid);
      p = p->next;    
    }
    printf("\n");
  }


//definiendo array de Queue
Queue** initializeQueues()
{
  
  int timeSlices[] = {20, 40, 60, 80, 100};
  Queue** queues = (Queue**)malloc(sizeof(Queue*)*QUEUES_AMOUNT);
  for (int i=0; i < QUEUES_AMOUNT; i++)
  {
    
    queues[i] = initialize(timeSlices[i]);
    
  }
  queue_initialized = 1;
  return queues;
}
/*typedef struct Queue
{
   proc_info_t* data;
   int last;
   int init;
   int maxCapacity;
} Queue;

void initalize(Queue* q)
{
  q->data = malloc(sizeof(proc_info_t) * q->maxCapacity);
  q->init = -1;
  q->last = -1;
  q->maxCapacity = 16;
}

void enque(Queue* q, proc_info_t proc) {
  if (q->last == q->maxCapacity - 1) {
    q->maxCapacity<<1;
    q->data = realloc(q->data, sizeof(proc_info_t) * q->maxCapacity);
  }
  q->data[++q->last] = proc;
}
*/

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
                   int curr_pid) {
  //(los procesos están ordenados por orden de llegada)
  //sjp devuelve el proceso al que le falte menos tiempo para finalizar
  // Información que puedes obtener de un proceso
  
  
  if (curr_pid != -1)//si esto pasa el proceso no ha finalizado su ejecucion
  {
    return curr_pid;
  }
  
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);
  int shortest_duration = process_total_time(procs_info[0].pid);
  int pid_to_return = procs_info[0].pid;
  int pos = 0;
  
  for (size_t i = 1; i < procs_count; i++)
  {
    int indexed_proc_duration = process_total_time(procs_info[i].pid);
      if (indexed_proc_duration < shortest_duration)
      {
       shortest_duration = indexed_proc_duration;
       pid_to_return = procs_info[i].pid;
       pos = i;
      }
    
  }
  
 /*
  procs_info[0] = procs_info[pos];
  procs_info[pos].executed_time = exec_time;
  procs_info[pos].on_io = on_io;
  procs_info[pos].pid = pid;
*/

  return pid_to_return;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  int shortest_completion_time = process_total_time(procs_info[0].pid) - procs_info[0].executed_time;
  int pid_to_return = procs_info[0].pid;
  for (size_t i = 1; i < procs_count; i++)
  {
    int indexed_proc_completion_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      if (indexed_proc_completion_time < shortest_completion_time)
      {
       shortest_completion_time = indexed_proc_completion_time;
       pid_to_return = procs_info[i].pid;
       
      }
    
  }
  
  return pid_to_return;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

 time_slice += 10;
 if (time_slice == 200 || time_slice == 0)
 {
  time_slice = 0;
  
  //termine y no estaba en ultima posicion
  if (curr_proc_pos <= procs_count-1 && curr_pid < 0)
  {
    //ejecuto el proceso que venía después(ahora está en mi posición poruqe terminé)
    return procs_info[curr_proc_pos].pid;
  }
  //no terminé y no estaba en última posición
  if (curr_proc_pos < procs_count-1)
  {
    //pasamos al siguiente proceso, que es indexable
    return procs_info[++curr_proc_pos].pid;
  }

  //estaba en última posición
  curr_proc_pos = 0;
  return procs_info[0].pid;
}
  if (curr_pid < 0)
  {
    return -1;
  }
  //mientras el proceso no termine y no llegue otro time slice se sigue ejecutando
  return procs_info[curr_proc_pos].pid;
}



int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  priority_boost_count += 10;
  
  queues = queue_initialized==1 ? queues : initializeQueues();
 
  
  
  
    

  //printf("*********     QUEUE      **************\n\n");

  //printQueue();

  //printf("*********     END QUEUE      **************\n\n");
 
  proc_info_t* process = dequeue(queues[current_queue]);
  if (process != NULL)
  {
  printf("Current PID=%d dequeue = %d \n", curr_pid,process->pid);
    /* code */
  }
  


  if (curr_pid >= 0 && process!=NULL )
  {//proceso no ha terminado entonces encolamos
    printf("Process data: process pid %d, process on_io %d, process exec_time: %d\n", process->pid, process->on_io, process->executed_time);
    int nextQueue = current_queue < QUEUES_AMOUNT - 1 ? current_queue + 1 : current_queue;
    if(process->executed_time >= queues[current_queue]->time_slice)
    {
      printf("ENQUE IN NEXT %d, process=%d\n", nextQueue, process->pid);
      enqueue(queues[nextQueue], process);
    }
    else
    {
      printf("EUQUE IN CURRENT %d, process=%d\n", current_queue, process->pid);
      enqueue(queues[current_queue], process);
    }
    
  }

  //incorporando procesos recien llegados a primera cola
  for (size_t i = 0; i < procs_count; i++)
  {
    if (procs_info[i].executed_time == 0 && procs_info[i].pid > last_pid_in_queue)
    {
      last_pid_in_queue = procs_info[i].pid;
      printf("last pid in queue %d\n", last_pid_in_queue);
      proc_info_t* reference = &procs_info[i];
      enqueue(queues[0], reference);
      printf("Enqueue done\n");
    }
  }
 printQueue(0);
  int result = curr_pid;
  for (int i = 0; i < QUEUES_AMOUNT; i++)
  {
    if (queues[i]->first != NULL)
    {
      current_queue = i;
      result = queues[i]->first->process->pid;
      printf("Result updated: %d \n", result);
      break;
    }
  }

  printf("Currente queue = %d\n",current_queue);
  // if (priority_boost_count >= priority_boost)
  // {
  //   priority_boost_count = -10;
  //   for (int i = QUEUES_AMOUNT - 1; i>=1; i--) {
  //     while (queues[i]->first!=NULL)
  //     {
  //       enqueue(queues[0], dequeue(queues[i]));
  //     }
  //   }
  // }
  return result;
}
  

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
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
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.
  printf("GETTING SCHEDULER %s \n",name);
  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;


  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}

