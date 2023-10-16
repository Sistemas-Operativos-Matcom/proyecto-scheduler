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

int lastprocess = -1;
int lastlastprocess = -1;
const int TimeSlice = 5;
int priority = 1000;

Queue_t First = {{},{}, 0, 0};
Queue_t Second = {{}, {}, 0, 0};
Queue_t Third = {{}, {}, 0, 0};

Queue_t empty ()
{
  Queue_t q = {{},{}, 0, 0};;
  return q;
}

int bool = 1;

int prr = 0;

const int Times = 5;
int times = Times;

void enqueue(Queue_t *queue, proc_info_t process) {
  queue->data[queue->rear] = process;
  queue->Time[queue->rear] = 0;
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

int its_over (int pid, proc_info_t *procs_info, int procs_count)
{
  for (size_t i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return 0;
    }
  }
  return 1;
}

int on_io (int pid, proc_info_t *procs_info, int procs_count)
{
  for (size_t i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return procs_info[i].pid;
    }
  }
  return -1;
}


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
  if (curr_pid != -1)
  {
    // se devuelve el pid del proceso actual dado que como es distinto de -1 no 
    // ha terminado de ejectutarse
    PID = curr_pid;
  }
  else
  {
    // se toma un pid valido (primer valor del array de procs_info)
    PID = procs_info[0].pid;
  }
  for (size_t i = 0; i < procs_count; i++)
  {
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
    // se toma un valor de pid y tiempo validos
    time = process_total_time(procs_info[0].pid);
    PID = procs_info[0].pid;
  }
  else
  {
    // este valor funciona como cota
    time = process_total_time(curr_pid);
  }
  for (size_t i = 0; i < procs_count; i++)
  {
    // compara con la cota y se queda con el pid del proceso de menor time to completion
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < time)
    {
      PID = procs_info[i].pid;
      time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    }
  }
  return PID;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  //time slice = 50
  if (times > 0 && curr_pid != -1)
  // devuelve el proceso anterior si aun no se ha cumplido el tiempo de cambio
  {
    times --;
    return curr_pid;
  }
  times = Times;
  if (curr_pid != -1)
  // si curr_pid = -1, prr no debe cambiar su valor ya que significa que el proceso 
  // anterior termino su ejecucion y por tanto sale del array de procesos
  {
    prr ++;
  }
  if (prr >= procs_count)
  // si prr se sale del array se comienza por el principio
  {
    prr = 0;
  }
  times --;
  return procs_info[prr].pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  if (curr_time % priority == 0 || bool)
  // vacia las colas (iguala la prioridad de todos los procesos)
  {
    First = empty();
    Second = empty();
    Third = empty();
    
    lastprocess = -1;
    lastlastprocess = -1;
    bool = 0;
  }
  for (int i = procs_count - 1; i > -1; i--)
  {
    if (procs_info[i].pid != lastprocess && procs_info[i].pid != lastlastprocess)
    // al iterar por el array procs_info de atras hacia adelante se annaden los procesos nuevos
    // para encontrar los procesos nuevos se compara con el ultimo pid que habia en el array en el 
    // timer interrupt anterior (se compara tambien con el penultimo ya que si el ultimo proceso fue el
    // que se ejecuto en el timer interrupt anterior y termino su ejecucion no se encontraria ese pid)
    {
      enqueue(&First,procs_info[i]);
    }
    else 
    {

      break;
    }
  }

  // actualizar los pid para comparar 
  lastprocess = procs_info[procs_count-1].pid;
  lastlastprocess = procs_info[procs_count-2].pid;

  if (First.front != First.rear) //cola no vacia
  {
    proc_info_t proc = get(&First); //obtener el primer elemento
    while (its_over(proc.pid, procs_info,procs_count))
    // se revisa que los procesos en cola no hayan terminado su ejecucion 
    {
      dequeue(&First);
      if (First.front != First.rear)
      {
        proc = get(&First);
      }
      else 
      {
        break;
      }
    }
    if (First.front != First.rear)
    {
      First.Time[First.front] ++; //aumenta el timepo que el proceso lleva en la cola
      if (First.Time[First.front] == TimeSlice)
      // si se cumple el time slice lo baja de prioridad
      {
        dequeue(&First);
        enqueue(&Second, proc);
        Second.Time[Second.rear] = 0;
      }
      //devolver el proceso
      return proc.pid;
    }
    
  } 

  if (Second.front != Second.rear)
  {
    proc_info_t proc = get(&Second);
    while (its_over(proc.pid, procs_info,procs_count))
    {
      dequeue(&Second);
      if (Second.front != Second.rear)
      {
        proc = get(&Second);
      }
      else 
      {
        break;
      }
    }
    if (Second.front != Second.rear)
    {
      Second.Time[Second.front]++;
      if (Second.Time[Second.front] == TimeSlice)
      {
        dequeue(&Second);
        enqueue(&Third, proc);
        Third.Time[Third.rear] = 0;
      }
      return proc.pid;
    }
    
  }

  if (Third.front != Third.rear)
  {
    proc_info_t proc = get(&Third);
    while (its_over(proc.pid, procs_info,procs_count))
    {
      dequeue(&Third);
      if (Third.front != Third.rear)
      {
        proc = get(&Third);
      }
      else 
      {
        break;
      }
    }
    if (Third.front != Third.rear)
    {
      Third.Time[Third.front]++;
      if (Third.Time[Third.front] == TimeSlice)
      // si se cumple el time slice mueve el elemento al final de la cola (hace funcion de rr)
      {
        dequeue(&Third);
        enqueue(&Third, proc);
        Third.Time[Third.rear] = 0;
      }
      return proc.pid;
    }
    
  }
  // si las tres colas estan vacias no hay procesos para ejecutar
  return -1;
}

int mlfq_IO_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  if (curr_time % priority == 0 || bool)
  {
    First = empty();
    Second = empty();
    Third = empty();
    
    lastprocess = -1;
    lastlastprocess = -1;
    bool = 0;
  }
  for (int i = procs_count - 1; i > -1; i--)
  {
    if (procs_info[i].pid != lastprocess && procs_info[i].pid != lastlastprocess)
    {
      enqueue(&First,procs_info[i]);
    }
    else 
    {
      
      break;
    }
  }
  int can = 1;
  lastprocess = procs_info[procs_count-1].pid;
  lastlastprocess = procs_info[procs_count-2].pid;

  if (First.front != First.rear)
  {
    proc_info_t first_process = get(&First);
    proc_info_t proc = first_process;
    while(on_io(proc.pid, procs_info,procs_count))
    {
      int time = First.Time[First.front];
      dequeue(&First);
      enqueue(&First,proc);
      First.Time[First.rear] = time;
      proc = get(&First);
      if (proc.pid == first_process.pid)
      {
        can = 0;
        break;
      }
    }
    while (its_over(proc.pid, procs_info,procs_count))
    {
      dequeue(&First);
      if (First.front != First.rear)
      {
        proc = get(&First);
      }
      else 
      {
        break;
      }
    }
    if (First.front != First.rear && can)
    {
      First.Time[First.front] ++;
      if (First.Time[First.front] == TimeSlice)
      {
        dequeue(&First);
        enqueue(&Second, proc);
        Second.Time[Second.rear] = 0;
      }
      printf("1");
      return proc.pid;
    }
    can = 1;
    
  } 

  if (Second.front != Second.rear)
  {
    proc_info_t first_process = get(&Second);
    proc_info_t proc = first_process;
    while(on_io(proc.pid, procs_info,procs_count))
    {
      int time = Second.Time[Second.front];
      dequeue(&Second);
      enqueue(&Second,proc);
      Second.Time[Second.rear] = time;
      proc = get(&Second);
      if (proc.pid == first_process.pid)
      {
        can = 0;
        break;
      }
    }
    while (its_over(proc.pid, procs_info,procs_count))
    {
      dequeue(&Second);
      if (Second.front != Second.rear)
      {
        proc = get(&Second);
      }
      else 
      {
        break;
      }
    }
    if (Second.front != Second.rear && can)
    {
      Second.Time[Second.front]++;
      if (Second.Time[Second.front] == TimeSlice)
      {
        dequeue(&Second);
        enqueue(&Third, proc);
        Third.Time[Third.rear] = 0;
      }
      printf("2");
      return proc.pid;
    }
    can = 1;
    
  }

  if (Third.front != Third.rear)
  {
    proc_info_t first_process = get(&Third);
    proc_info_t proc = first_process;
    while(on_io(proc.pid, procs_info,procs_count))
    {
      int time = Third.Time[Third.front];
      dequeue(&Third);
      enqueue(&Third,proc);
      Third.Time[Third.rear] = time;
      proc = get(&Third);
      if (proc.pid == first_process.pid)
      {
        can = 0;
        break;
      }
    }
    while (its_over(proc.pid, procs_info,procs_count))
    {
      dequeue(&Third);
      if (Third.front != Third.rear)
      {
        proc = get(&Third);
      }
      else 
      {
        break;
      }
    }
    if (Third.front != Third.rear && can)
    {
      Third.Time[Third.front]++;
      if (Third.Time[Third.front] == TimeSlice)
      {
        dequeue(&Third);
        enqueue(&Third, proc);
        Third.Time[Third.rear] = 0;
      }
      printf("3");
      return proc.pid;
    }
    can = 1;
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
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_IO_scheduler;
  
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
