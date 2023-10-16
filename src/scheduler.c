#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h> // para utilizar el tipo bool

#include "simulation.h"

int TIME_SLICE;
int accu_time_rr;
int proc_index;
struct Queue* highest_priority;
struct Queue* h_priority_time;
struct Queue* second_priority;
struct Queue* s_priority_time;
struct Queue* lowest_priority;
struct Queue* l_priority_time;
int PRIORITY_BUS_PERIOD;
int accu_time_mlfq;
int previous_pid;



/////////////////////////////////////////////////////////////////////////////
// Definición y métodos de la estructura que modela el comportamiento de 
// una cola para la estrategia MLFQ
struct Queue {
    int front, tail, size;
    unsigned capacity;
    int* array;
};

struct Queue* Create_Queue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    queue->tail = 0;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    
    return queue;
}
int is_Full(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
int is_Empty(struct Queue* queue)
{
    return (queue->size == 0);
}
void enqueue(struct Queue* queue, int item)
{
    if(is_Full(queue)) {        
        return;
    }    
    queue->array[queue->tail] = item;
    queue->tail = queue->tail + 1;
    queue->size = queue->size + 1;    
}
int dequeue(struct Queue* queue)
{
    if(is_Empty(queue)) 
    {
        return -1;
    }
    int item = queue->array[queue->front];
    for(int i = 1; i < queue->tail; i++) {
        queue->array[i-1] = queue->array[i];
        queue->array[i] = -1;
    }   
    
    queue->tail = queue->tail - 1;
    queue->size = queue->size - 1;
    
    return item;
}
int front(struct Queue* queue)
{
    return queue->array[queue->front];
}
// Verifica la existencia de un elemento en la estructura
// Si el elemento se encuentra en la estructura, devuelve la posición
// que le corresponde en el array interno, 
// en caso contrario, devuelve -1
int in_Queue(struct Queue* queue, int item)
{
    if(is_Empty(queue)) {
        return -1;
    }
    for(int i = 0; i < queue->tail; i++)
    {
        if(item == queue->array[i]){
            return i;
        }
    }
    return -1;
}
// Elimina el elemento cuya posición en el array interno de la estructura es
// igual al valor pos
void remove_from_Queue(struct Queue* queue,int pos)
{
    if(is_Empty(queue)){
        return;
    }
    // guardar el valor en la cabecera de la estructura
    int first_element = queue->array[queue->front];
    // Si pos es igual a la posicion del elemento en la cabecera de la cola
    if(in_Queue(queue,first_element) == pos)
    {
        dequeue(queue);
        return;
    }
    // valor del elemento a eliminar 
    int item = queue->array[pos];

    while(front(queue) != item){
        enqueue(queue,dequeue(queue));
    }
    dequeue(queue);
    // restaurar el estado inicial de la estructura    
    while(front(queue) != first_element){
        enqueue(queue,dequeue(queue));
    }
}
/////////////////////////////////////////////////////////////////////////////


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
    // Verificaciones 
  //  printf("Cantidad de procesos activos: %d \n",procs_count);
   // valor del pid del proceso actual cuando ha finalizado
  //  printf("PID del proceso actual: %d\n", curr_pid);
   // si el proceso actual ha finalizado, su pid no se encuentra en la lista de procesos activos
  //  for( int i = 0; i < procs_count; i++) {
  //   if(procs_info[i].pid == curr_pid) {
  //     printf("El proceso está en la lista \n");
  //     printf("Tiempo de ejecución del proceso actual: %d \n",procs_info[i].executed_time);  
  //   }
  //  }  
   
  //  printf("NEXT PID: %d \n",procs_info[0].pid);
  //  printf("Tiempo de la simulación: %d \n",curr_time);
  
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Devuelve el pid del proceso actual hasta que termine su tiempo de ejecución
  if(curr_pid != -1) {
    return curr_pid;
  }
  // PID del proceso de menor tiempo de duración
  int shortest_job_pid = procs_info[0].pid; 
  
  for(int j = 1; j < procs_count; j++) {
    // duración del proceso j-ésimo de la lista de procesos activos  
     int duration = process_total_time(procs_info[j].pid);
    // Si el proceso j-ésimo tiene un tiempo de duración menor que el proceso de menor duración encontrado hasta ahora
    // actualiza el PID del proceso de menor duración 
     if(duration < process_total_time(shortest_job_pid)) {       
       shortest_job_pid = procs_info[j].pid;
     }    
  }   
  // Se devuelve el PID del proceso de menor duración entre todos los procesos activos 
  return shortest_job_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
    // PID del primer proceso activo en la lista de procesos activos
    int next_pid = procs_info[0].pid;
    // Tiempo restante para la ejecución del primer proceso de la lista
    int shortest_time_to_complete =  process_total_time(next_pid) - procs_info[0].executed_time;
    
    // Buscar el proceso activo que menor tiempo le quede para completar su ejecución
    for(int i = 1; i < procs_count; i++){
      // Tiempo restante para la ejecución del proceso i-ésimo 
      int time_to_complete_job = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      // Si el tiempo restante para la ejecución del proceso i-ésimo es menor que el valor de la variable
      // shortest_time to complete; actualizar el valor de next_pid y shortest_time_to_complete
      if(time_to_complete_job < shortest_time_to_complete) {
        next_pid = procs_info[i].pid;
        shortest_time_to_complete = time_to_complete_job;
      }
    }
  // Devuelve el PID del proceso al que le queda menos tiempo de ejecución
  return next_pid;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // estado inicial de la simulación donde no se ha ejecutado ningún proceso
  if(curr_pid < 0 && accu_time_rr == 0) {       
    return procs_info[proc_index].pid;
  }
  accu_time_rr = accu_time_rr + 10; // la constante depende del TIMER_INTERRUPT_PERIOD_MS definido en simulation.c

  // valor true si se ha consumido un tiempo igual a un múltiplo del TIME_SLICE 
  bool change = (accu_time_rr != 0) && (accu_time_rr % TIME_SLICE) == 0 ;
  
  // si se ha consumido un tiempo igual a un múltiplo SLICE_TIME y el próximo valor del puntero 
  // coincide con procs_count
  if(change && proc_index + 1 == procs_count) {    
    proc_index = 0;
    return procs_info[proc_index].pid;
  }
  if(change){    
    proc_index = proc_index + 1;
    return procs_info[proc_index].pid;
  }
  // si la cantidad de procesos activos coincide con el valor del puntero
  if(proc_index == procs_count){
    proc_index = 0;
  }
  // si el proceso actual ha terminado, 
  // cambiar de proceso sin actualizar el valor del puntero
  if(curr_pid < 0) {
    return procs_info[proc_index].pid;
  }  
  // devuelve el PID del proceso actual
  return curr_pid;
}

/////////////////////////////////////////////////////////////////////////////
// Métodos auxiliares para la estrategia MLFQ

// Actualiza la cola de mayor prioridad cuando llegan procesos nuevos 
void fill_highest_priority(proc_info_t *procs_info, int procs_count)
{
  // si las tres colas estan vacias
  // agregar el PID de cada proceso activo en la cola de mayor prioridad
  if(is_Empty(highest_priority) && (is_Empty(second_priority) && is_Empty(lowest_priority))){
    for(int i = 0; i < procs_count; i++){
      int item = procs_info[i].pid;
      int time = procs_info[i].executed_time;
      enqueue(highest_priority,item);
      enqueue(h_priority_time,time);
    }
    return;
  }

  for(int i = 0; i < procs_count; i++){
      int item = procs_info[i].pid;
      // si el proceso i-ésimo de la lista de procesos activos se encuentra en alguna      
      if(in_Queue(second_priority, item) != -1 || in_Queue(lowest_priority,item) != -1){
        continue;
      }
      // Si el PID del proceso i-ésimo no se encuentra en la cola de mayor prioridad
      // agregarlo a la misma
      if(in_Queue(highest_priority,item) == -1){
        int time = procs_info[i].executed_time;
        enqueue(highest_priority,item);
        enqueue(h_priority_time,time);
      }
    }
}
// Actualiza el tiempo que de ejecución de cada proceso activo en la cola correspondiente
void update_queues(proc_info_t *procs_info, int procs_count)
{
  for(int i = 0; i < procs_count; i++){
    int item = procs_info[i].pid;
    int time = procs_info[i].executed_time;
    int pos = -1;
    int time_gap = 0;
    if(in_Queue(highest_priority, item) != -1){
      pos = in_Queue(highest_priority,item);
      time_gap = time - h_priority_time->array[pos];

      if(time_gap > 0){
        h_priority_time->array[pos] = h_priority_time->array[pos] + time_gap;
      }
      if(h_priority_time->array[pos] >= TIME_SLICE){
        remove_from_Queue(highest_priority, pos);
        remove_from_Queue(h_priority_time, pos);

        enqueue(second_priority, item); 
        enqueue(s_priority_time, time);
      }
            
    } else if(in_Queue(second_priority, item) != -1){
      pos = in_Queue(second_priority,item);
      time_gap = time - s_priority_time->array[pos];

      if(time_gap > 0){
        s_priority_time->array[pos] = s_priority_time->array[pos] + time_gap;
      }
      if(s_priority_time->array[pos] >= TIME_SLICE){
        remove_from_Queue(second_priority, pos);
        remove_from_Queue(s_priority_time, pos);

        enqueue(lowest_priority, item); 
        enqueue(l_priority_time, time);
      }      
      } else if(in_Queue(lowest_priority,item) != -1){
        pos = in_Queue(lowest_priority,item);
        time_gap = time - l_priority_time->array[pos];

        if(time_gap > 0){
          l_priority_time->array[pos] = l_priority_time->array[pos] + time_gap;
        }
        if(l_priority_time->array[pos] >= TIME_SLICE){
          int pid = lowest_priority->array[pos];
          remove_from_Queue(lowest_priority,pos);
          int time_temp = l_priority_time->array[pos];
          remove_from_Queue(l_priority_time,pos);
          enqueue(lowest_priority,pid); 
          enqueue(l_priority_time,time_temp);
        }      
      }    
  }
}
// Cuando se consume un tiempo igual a un múltiplo del PRIORITY_BUS_PERIOD
// actualiza el estado de la cola de mayor prioridad, insertando en la misma
// cada elemento de las colas restantes, según su prioridad
void priority_update()
{
  while(is_Empty(second_priority) != 1){
    enqueue(highest_priority, dequeue(second_priority));
  }
  while(is_Empty(lowest_priority) != 1){
    enqueue(highest_priority, dequeue(lowest_priority));
  }
}
/////////////////////////////////////////////////////////////////////////////

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  accu_time_mlfq = accu_time_mlfq + 10;  // la constante depende del TIMER_INTERRUPT_PERIOD_MS definido en simulation.c

  // si las tres colas estan vacías y curr_pid = -1
  if(curr_pid == -1 && (is_Empty(highest_priority) && (is_Empty(second_priority) && is_Empty(lowest_priority)))){
    fill_highest_priority(procs_info,procs_count);    
    previous_pid = front(highest_priority);
    return front(highest_priority);
  }  
  // Si el proceso actual terminó su ejecución, eliminar el PID del proceso en la cola correspondiente
  if(curr_pid == -1){
    if(in_Queue(highest_priority,previous_pid) == 0)
    {
      dequeue(highest_priority);
      dequeue(h_priority_time);
    } else if(in_Queue(second_priority,previous_pid) == 0){
      dequeue(second_priority);
      dequeue(s_priority_time);
    }else if(in_Queue(lowest_priority,previous_pid) == 0){
      dequeue(lowest_priority);
      dequeue(l_priority_time);
    }
  }

  // Guardar el valor de la cabecera de cada cola como estado previo 
  // a que se actualice su estado  
  int h_front = front(highest_priority);  
  int s_front = front(second_priority);  
  int l_front = front(lowest_priority);
  

  // Actualizar el estado de cada cola, teniendo en cuenta el tiempo 
  // de  ejecución de cada proceso activo en su respectiva cola
  update_queues(procs_info, procs_count);
  // Si han llegado procesos nuevos, insertar el PID de cada uno en la cola de mayor prioridad
  fill_highest_priority(procs_info,procs_count);
  // Si se ha consumido un tiempo igual a un múltiplo del valor PRIORITY_BUS_PERIOD
  // posicionar el PID de cada proceso en la cola de mayor prioridad
  if(accu_time_mlfq % PRIORITY_BUS_PERIOD == 0){
    priority_update();
  }
  // Seleccionar el PID del proceso a ejecutar
  int next_pid = 0;
  // Se determina la cola con mayor prioridad que tiene al menos un elemento
  // Si el valor en la cabacera de la cola no cambió después de actualizar su estado y
  // se ha consumido un tiempo igual a un múltiplo del TIME_SLICE
  // entonces se desplaza el valor de la cabecera para el final de la cola (simulando el efecto de Round_Robin)
  // actualiza el valor next_pid
  if(is_Empty(highest_priority) == 0)
  {
    if( (h_front == front(highest_priority)) && (accu_time_mlfq % TIME_SLICE == 0))
    {
      enqueue(highest_priority, dequeue(highest_priority));
      enqueue(h_priority_time, dequeue(h_priority_time));      
    }
    next_pid = front(highest_priority);
  } else if(is_Empty(second_priority) == 0){
    if( (s_front == front(second_priority)) && (accu_time_mlfq % TIME_SLICE == 0))
    {
      enqueue(second_priority, dequeue(second_priority));
      enqueue(s_priority_time, dequeue(s_priority_time));
    }
    next_pid = front(second_priority);    
  } else if(is_Empty(lowest_priority) == 0){
    if(l_front == front(lowest_priority) && (accu_time_mlfq % TIME_SLICE == 0))
    {
      enqueue(lowest_priority, dequeue(lowest_priority));
      enqueue(l_priority_time, dequeue(l_priority_time));
    }
    next_pid = front(lowest_priority);
  }

  previous_pid =  next_pid;
  // Devuelve el PID del proceso actual si no se ha consumido un 
  // tiempo igual a TIME_SLICE
  return next_pid;
}


// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  // Constante definida para las estrategias Round Robin y MLFQ
  TIME_SLICE = 50;
  // múltiplo del TIMER_INTERRUPT_PERIOD, se actualiza en cada timer_interrupt
  accu_time_rr = 0;
  // puntero que indica el proceso de la lista procs_info a ejecutar
  proc_index = 0;
  if (strcmp(name, "rr") == 0) return *round_robin_scheduler;

  // Inicializar las tres colas que se utilizan en la estrategia MLFQ
  // cada cola almacena los PID de los procesos activos segun corresponda;
  // por cada una se inicializa una cola de igual capacidad para medir
  // el tiempo de ejecución de cada proceso en la cola
  highest_priority = Create_Queue(10000);
  h_priority_time = Create_Queue(10000);

  second_priority = Create_Queue(10000);
  s_priority_time = Create_Queue(10000);

  lowest_priority = Create_Queue(10000);
  l_priority_time = Create_Queue(10000);

  PRIORITY_BUS_PERIOD = 100;
  // múltiplo del TIMER_INTERRUPT_PERIOD, se actualiza en cada timer_interrupt
  accu_time_mlfq = 0;
  // guarda el PID del proceso que se ejecutó en el timer_interrupt anterior
  previous_pid = 0;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
