#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"

const QUEUES_AMOUNT = 5;
int curr_proc_pos = 0;
int time_slice_count = 0;
int time_slice_for_rr = -10;
int last_pid_in_queue = -1;
int queue_initialized = 0;
int current_queue = 0;
int priority_boost_count = -10;
int RR_time_slice = 60;
int priority_boost = 500;
int doing_RR = 0;
RR_pid = 0;
int* procs_indexer;
int* ptr;
Queue** queues;

printQueue() {
  for (int i=0;i<QUEUES_AMOUNT;i++)
  {

  ProcessNode* p = queues[i]->first;
  if (p != NULL) {
      printf("QUEUE #%d:", i);
      do  
      {
        printf("%d , ",p->process.pid);
        p = p->next;    
      } while (p != NULL);
      printf("\n");
  }
  }
}


//definiendo array de Queue y el procs_indexer
Queue** initializeQueues()
{
  procs_indexer = (int*)malloc(MAX_PROCESS_COUNT*sizeof(int));
  for (int i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    procs_indexer[i] = i;

  }
  
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
                     int curr_pid) 
{
//si la pos es válida mando a ejecutarse al proceso en esa pos hasta alcanzar time slice,
//si proc termina time slice = 10
//si pos no es válida mandar a pos 0
if (RR_time_slice != time_slice_count)
{
  time_slice_count +=10;
  if (curr_proc_pos <= procs_count-1)
  { if (curr_pid < 0)
  {
    time_slice_count = 10;
  }
    return procs_info[curr_proc_pos].pid;
  }
  
}
 time_slice_count = 10;
 if (curr_proc_pos >= procs_count-1)
 {
  curr_proc_pos = 0;
  return procs_info[0].pid;
 }
 return procs_info[++curr_proc_pos].pid;
}

//  if (curr_pid != -1 && (time_slice_count != 40 || time_slice_count !=0))
//  {
//   //proceso se sigue ejecutando
//   time_slice_count += 10;
//   return curr_pid;
//  }
//  return NextProcessAcordingToRR(procs_info,procs_count,curr_time,curr_pid);
// }
//    int NextProcessAcordingToRR(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid)
//    {
//       time_slice_count = 0;
//         if (curr_proc_pos <= procs_count-1 && curr_pid < 0)
//       {//termine y no estaba en ultima posicion
//         //ejecuto el proceso que venía después(ahora está en mi posición porque terminé)
//         return procs_info[curr_proc_pos].pid;
//       }
//    }
  
//   //no terminé y no estaba en última posición
//   if (curr_proc_pos < procs_count-1)
//   {
//     //pasamos al siguiente proceso, que es indexable
//     return procs_info[++curr_proc_pos].pid;
//   }

//   //estaba en última posición
//   curr_proc_pos = 0;
//   return procs_info[0].pid;
// }
//   if (curr_pid < 0)
//   {
//     return -1;
//   }
//   //mientras el proceso no termine y no llegue otro time slice se sigue ejecutando
//   return procs_info[curr_proc_pos].pid;
// }




int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

  //procesos que lleguen nuevos se ponen en primera cola
  //se manda a ejecutar el de mayor prioridad, si hay varios se ejecutan según RR
  //si el proceso termina o se pone en io se manda otro proceso
  //para bajar de las colas se toma en cuenta solo el cpu time de cada proceso
  //un proceso no puede terminar en io,siempre viene un CPU time despues
  priority_boost_count += 10;
 

  queues = queue_initialized==1 ? queues : initializeQueues();

 if (doing_RR && time_slice_count < RR_time_slice && !(curr_pid < 0 || procs_info[procs_indexer[curr_pid]].on_io) && priority_boost_count < priority_boost)
 {
  time_slice_count+=10;
  queues[current_queue]->first->process.executed_time += 10;
  return curr_pid;
 }
  time_slice_count = 0;
  proc_info_t* process = dequeue(queues[current_queue]);
 
  if (curr_pid >= 0)
  {//proceso no ha terminado entonces encolamos
    
      //printf("Process data: process pid %d, process on_io %d, process exec_time: %d\n", process->pid, process->on_io, process->executed_time);
    
    int nextQueue = current_queue < QUEUES_AMOUNT - 1 ? current_queue + 1 : current_queue;
    if(process->executed_time >= queues[current_queue]->time_slice)
    {
      enqueue(queues[nextQueue], *process);
    }
    else
    {
      enqueue(queues[current_queue], *process);
      
    }
    
  }
//incorporando procesos recien llegados a primera cola
  for (size_t i = 0; i < procs_count; i++)
  {
    if (procs_info[i].executed_time == 0 && procs_info[i].pid > last_pid_in_queue)
    {
      last_pid_in_queue = procs_info[i].pid;
      proc_info_t reference = procs_info[i];
      enqueue(queues[0], reference);
      
    }
  }
  int result = curr_pid;
 //haciendo priority boost
 //printf("Before pb %d.\n", priority_boost_count);
  if (priority_boost_count >= priority_boost)
  {
   
    priority_boost_count = 0;
    current_queue = 0;
    for (int i = QUEUES_AMOUNT - 1; i>=1; i--) {
      while (queues[i]->first!=NULL)
      {
        proc_info_t* p = dequeue(queues[i]);
        enqueue(queues[0], *p);
      }
      
    }
  }
//actualizando procs_indexer
  if (process != NULL && curr_pid < 0)
{ //printf("Process finished\n");
  //termino un proceso
  for (size_t i = process->pid + 1; i < MAX_PROCESS_COUNT; i++)
  {
    procs_indexer[i] -= 1;
  }
  
}

  //buscando primer elemento valido entre las colas segun la prioridad
  for (int i = 0; i < QUEUES_AMOUNT; i++)
  {  //encontrando primer proceso en cola i-esima que no este en io
      int first_element_pid = -1;
      int first_set = 0;
      int valid = 0;

      if (queues[i]->first != NULL)
      {
        while (first_element_pid != queues[i]->first->process.pid)
        {
          if (!first_set)
        {
          first_element_pid = queues[i]->first->process.pid;
          first_set = 1;
        }
        int a = queues[i]->first->process.pid;
        int index = procs_indexer[a];
        if(!procs_info[index].on_io)
        {
          valid = 1;
          break;
        }
         process = dequeue(queues[i]);
         enqueue(queues[i], *process);
        }
      
      if (!valid)
      {
        continue;
      }
      
      if (current_queue != i || RR_pid < 0)
      {
        doing_RR = 0;
      }
      current_queue = i;
      result = queues[i]->first->process.pid;
      if (result == RR_pid)
      {
       RR_pid = -1;
      }
      doing_RR = queues[current_queue]->first != queues[current_queue]->last;
      time_slice_count +=10;
      
      
      //printf("\n%d Este es doing_RR.\n", doing_RR);
      RR_pid = queues[current_queue]->last;

      //printf("Result updated: %d \n", result);
      break;
     }
    }
    queues[current_queue]->first->process.executed_time += 10;
    return result;
  }
      


      // while (queues[i]->first != NULL)
      // { printf("first node of queue %d is: %d\n", i,queues[i]->first->process.pid);
      //   int a = queues[i]->first->process.pid;
      //   int index = procs_indexer[a];
      //   if (!first_set)
      //   {
      //     first_element_pid = queues[i]->first->process.pid;
      //     first_set = 1;
      //   }
      //   if (first_set == 1 && first_element_pid == a)
      //   {
      //     continue;
      //   }
        
      //   if(!procs_info[index].on_io)
      //   {
      //    break;
      //   }
        
      //    process = dequeue(queues[i]);
      //   //  if (queues[i]->first != NULL)
      //   //  {
      //   //   printf("first node of queue %d is: %d\n", i,queues[i]->first->process.pid);
      //   //  }
         
      //    enqueue(queues[i], *process);
      // }
      
    // if (queues[i]->first != NULL && !procs_info[procs_indexer[first_element_pid]].on_io)
    //  {
    //   if (current_queue != i || RR_pid < 0)
    //   {
    //     doing_RR = 0;
    //   }
    //   current_queue = i;
    //   result = queues[i]->first->process.pid;
    //   if (result == RR_pid)
    //   {
    //    RR_pid = -1;
    //   }
    //   doing_RR = queues[current_queue]->first != queues[current_queue]->last;
    //   time_slice_count +=10;
      
      
    //   //printf("\n%d Este es doing_RR.\n", doing_RR);
    //   RR_pid = queues[current_queue]->last;

    //   //printf("Result updated: %d \n", result);
    //   break;
    //  }

//   }
//   queues[current_queue]->first->process.executed_time += 10;
//   return result;
// }
  

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

