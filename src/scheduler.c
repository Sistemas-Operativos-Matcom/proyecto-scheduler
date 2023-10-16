#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

#define num_colas 3
Queue colas_prioridades [num_colas];
int last_pid;

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
  int pid = curr_pid;   
  if(curr_pid==-1) 
  {     
    pid = procs_info[0].pid;
    int min_process = process_total_time(pid);  
    for(int i=1 ; i<procs_count ; i++)               
    {
      int new_pid = procs_info[i].pid;
      int new_time = process_total_time(new_pid);
      if(new_time < min_process)
      {
        min_process = new_time;
        pid = new_pid;
      }
    }
  }
  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  int pid = procs_info[0].pid;          
  int min_exec_process = process_total_time(pid) - procs_info[0].executed_time; 
  for(int i=1 ; i<procs_count ; i++)              
  {
    int new_pid = procs_info[i].pid;
    int new_exec_time = process_total_time(new_pid) - procs_info[i].executed_time;
    if(new_exec_time < min_exec_process)
    {
      min_exec_process = new_exec_time;
      pid = new_pid;
    }
  }
  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  int pid = curr_pid;
  if(pid==-1) return procs_info[0].pid;
  if(curr_time % 3 == 0)
  for(int i=0 ; i<procs_count ; i++)
  {
    if(pid==procs_info[i].pid)
    {
      if(i==procs_count-1) i=-1;
      while(procs_info[i+1].on_io!=0 && i<procs_count-2){
        i++;
      }
      pid = procs_info[i+1].pid;
      break;
    }
  }
  return pid;
}

int is_empty(Queue *q) {
  return q->rear == -1;
}

void enqueue(Queue *q,int value) {
  if(q->front == -1) q->front = 0;
  q->rear++;
  q->items[q->rear] = value;
}

int dequeue(Queue *q) {
  if(is_empty(q)!=1)
  {
    int value = q->items[q->front];
    if(q->front >= q->rear)
    {
      q->front = -1;
      q->rear = -1;
    }else q->front++;
    return value;
  }
  return 0;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(curr_pid==-1)
  {
    int last_pid_queue = find_queue_curr_pid(last_pid);  
    update_queue(&colas_prioridades[last_pid_queue],last_pid);
  }
  int pid= curr_pid;
  int actual = num_colas;
  int pos =-1;
  if(procs_count!=0) find_news(procs_info,procs_count);             // busco en el array si hay nuevos procesos y los agrego a la primera cola,que es la de mayor prioridad
  if(curr_pid!=-1)
  {
    actual = find_queue_curr_pid(curr_pid);     // busco en que cola esta el proceso que se esta ejecutando actualmente
    pos = find_pos(procs_info,curr_pid,procs_count);     //busco la posicion del proceso actual en el array de procesos
  }
  for(int i=0;i<actual;i++)             //recorro las colas con mayor prioridad al proceso actual ya que dichos procesos van primero
  {
    if(!is_empty(&colas_prioridades[i]))     //al encontrar una que no este vacia, el primer proceso en ella es el de mayor prioridad
    {
      int aux = (&colas_prioridades[i])->front;
      pid = (&colas_prioridades[i])->items[aux];
      pos = find_pos(procs_info,pid,procs_count);
      actual = i;
      while(procs_info[pos].on_io==1 && pos<procs_count)
      {
        pos++;
        actual = find_queue_curr_pid(procs_info[pos].pid);
      }
      break;                                  //no busco mas ya que ese debe ser el que se ejecute
    }
  }
  if(procs_info[pos].executed_time % 30 == 0 && pid == curr_pid && pos!=-1)        //si no encontre a nadie en colas anteriores procedo a ejecutar como RR
  {
    if(pos==procs_count-1 && (&colas_prioridades[actual])->front != (&colas_prioridades[actual])->rear)    //si era el ultimo de esa cola, vuelve a empezar por ella
    {
      int aux = (&colas_prioridades[actual])->front;
      pid = (&colas_prioridades[actual])->items[aux];
    }
    else if(pos < procs_count-1)                                                  //sino sigue con el siguiente
    pid = procs_info[pos+1].pid;
  }
  if(procs_info[pos].executed_time + 10 % 150 == 0 && actual!=num_colas-1)        //si estas proximo a cumplir un slice time de ejecucion, entonces te ejecutas y bajas de prioridad
  {
    pid = dequeue(&colas_prioridades[actual]);
    enqueue(&colas_prioridades[actual+1],pid);
  }
  last_pid = pid;
  return pid;            
}

void update_queue(Queue *q,int pid)
{
  for(int i= q->front;i<=q->rear;i++)
  {
    if(q->items[i]==pid)
    {
      for(int j=i;j<q->rear;j++)
      {
        q->items[j] = q->items[j+1];
      }
      q->rear--;
      return;
    }
  }
}

void find_news(proc_info_t *procs_info,int procs_count)
{
  int flag;
  for(int j=procs_count-1;j>=0;j--)
  {
    flag = 0;
    for(int z=0;z<num_colas;z++)
    {
      if(!is_empty(&colas_prioridades[z]))
      for(int i=(&colas_prioridades[z])->rear;i>=(&colas_prioridades[z])->front;i--)
      {
        if((&colas_prioridades[z])->items[i]==procs_info[j].pid) flag = 1;
      }
      if(flag==1) return;
    }
    enqueue(&colas_prioridades[0],procs_info[j].pid);
  }
}

int find_queue_curr_pid(int curr_pid)
{
  for(int i=0;i<num_colas;i++)
  {
    for(int j = (&colas_prioridades[i])->front; j<= (&colas_prioridades[i])->rear; j++)
    {
      if((&colas_prioridades[i])->items[j]==curr_pid) return i;
    }
  }
  return -1;
}

int find_pos(proc_info_t *procs_info,int pid,int procs_count)
{
  for(int i=0;i<procs_count;i++)
  {
    if(procs_info[i].pid == pid) return i;
  }
  return -1;
}

// Información que puedes obtener de un proceso
  //int pid = procs_info[0].pid;      // PID del proceso
  //int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  //int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  //int duration = process_total_time(pid);


// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  Queue cola1 = { .front = -1, .rear = -1, .items = {0} };
  Queue cola2 = { .front = -1, .rear = -1, .items = {0} };
  Queue cola3 = { .front = -1, .rear = -1, .items = {0} };

  colas_prioridades[0] = cola1;
  colas_prioridades[1] = cola2;
  colas_prioridades[2] = cola3;

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
