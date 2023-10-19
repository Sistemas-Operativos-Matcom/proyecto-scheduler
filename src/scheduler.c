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
struct Queue* interactive_proc;
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

// Debugueando MLFQ
void total_size(int procs_count, int curr_time)
{
  printf("Size de highest_priority: %d \n", highest_priority->size);
  printf("Size de second_priority: %d \n", second_priority->size);
  printf("Size de lowest_priority: %d \n", lowest_priority->size);
  printf("Cantidad de procesos activos: %d \n", procs_count);
  printf("Cantidad de elementos encolados: %d \n", (highest_priority->size + second_priority->size + lowest_priority->size));
  printf("Tiempo actual: %d \n", curr_time);
  printf("============================================ \n");
}

// Actualiza la cola de mayor prioridad cuando llegan procesos nuevos 
void fill_highest_priority(struct Queue* queue, proc_info_t *procs_info, int procs_count)
{
  for(int i = 0; i < procs_count; i++){
      int item = procs_info[i].pid;
      // si el proceso i-ésimo de la lista de procesos activos se encuentra en alguna      
      if(in_Queue(second_priority, item) != -1 || in_Queue(lowest_priority,item) != -1){
        continue;
      }
      // Si el PID del proceso i-ésimo no se encuentra en la cola de mayor prioridad
      // agregarlo a la misma
      if(in_Queue(highest_priority,item) == -1){        
        enqueue(highest_priority,item);
        enqueue(h_priority_time,0);
      }
    }
}
// Dado el PID de un proceso verifica si se encuentra en la lista de procesos activos
int is_active(proc_info_t *procs_info, int procs_count, int PID)
{
  for(int i = 0; i < procs_count; i++)
  {
    if(PID == procs_info[i].pid)
    {
      return 1;
    }   
  }
  return 0;
}
// Actualiza el tiempo que de ejecución de cada proceso activo en la cola correspondiente
void update_queues(proc_info_t *procs_info, int procs_count)
{
  // Si la cantidad de proceso activos es distinta de la cantidad de elementos que hay en las tres colas juntas
  if(procs_count != (highest_priority->size + second_priority->size + lowest_priority->size))
  {
    int pid = 0;
    int position = 0;
    // Buscar en cada cola, el PID correspondiente a un proceso que no se encuentra en la lista de procesos activos
    for(int p = 0; p < highest_priority->size; p++)
    {
      pid = highest_priority->array[p];
      if(!is_active(procs_info,procs_count, pid))
      {
        position = in_Queue(highest_priority, pid);
        remove_from_Queue(highest_priority, position);
        remove_from_Queue(h_priority_time, position);
      }       
    }

    // cola de segunda prioridad
    for(int r = 0; r < second_priority->size; r++)
    {
      pid = second_priority->array[r];
      if(!is_active(procs_info, procs_count, pid))
      {
        position = in_Queue(second_priority, pid);
        remove_from_Queue(second_priority, position);
        remove_from_Queue(s_priority_time, position);
      }       
    }
    // cola de menor prioridad
    for(int s = 0; s < lowest_priority->size; s++)
    {
      pid = lowest_priority->array[s];
      if(!is_active(procs_info, procs_count,pid))
      {
        position = in_Queue(lowest_priority, pid);
        remove_from_Queue(lowest_priority, position);
        remove_from_Queue(l_priority_time, position);
      }       
    }

  }
  
  for(int i = 0; i < procs_count; i++)
  {
    int item = procs_info[i].pid;    
    int pos = -1;    
    if(in_Queue(highest_priority, item) != -1)
    {    
      pos = in_Queue(highest_priority,item);        
      if(h_priority_time->array[pos] > 0 && h_priority_time->array[pos] % TIME_SLICE == 0)
      {        
        remove_from_Queue(highest_priority, pos);
        remove_from_Queue(h_priority_time, pos);

        enqueue(second_priority, item); 
        enqueue(s_priority_time, 0);
      }
            
    } else if(in_Queue(second_priority, item) != -1)
    {
      pos = in_Queue(second_priority, item);      
      if(s_priority_time->array[pos] > 0 && s_priority_time->array[pos] % TIME_SLICE == 0){
        
        remove_from_Queue(second_priority, pos);
        remove_from_Queue(s_priority_time, pos);

        enqueue(lowest_priority, item); 
        enqueue(l_priority_time, 0);
      }      
      } else if(in_Queue(lowest_priority,item) != -1)
      {
        // Un proceso que consuma un tiempo igual a SLICE_TIME en la cola de menor prioridad
        // Se desplaza al final de la cola, y se reestablece el tiempo que lleva en la cola a 0
        pos = in_Queue(lowest_priority,item);
        
        if(l_priority_time->array[pos] > 0 &&  l_priority_time->array[pos] % TIME_SLICE == 0){
          
          remove_from_Queue(lowest_priority,pos);          
          remove_from_Queue(l_priority_time,pos);

          enqueue(lowest_priority,item); 
          enqueue(l_priority_time, 0);
        }      
      }    
  }
}
// Cuando se consume un tiempo igual a un múltiplo del PRIORITY_BUS_PERIOD
// actualiza el estado de la cola de mayor prioridad, insertando en la misma
// cada elemento de las colas restantes, según su prioridad
void priority_update()
{
  while(is_Empty(second_priority) == 0){
    enqueue(highest_priority, dequeue(second_priority));
    enqueue(h_priority_time, dequeue(s_priority_time));
  }
  while(is_Empty(lowest_priority) == 0){
    enqueue(highest_priority, dequeue(lowest_priority));
    enqueue(h_priority_time, dequeue(l_priority_time));
  }
}
// Actualiza la cola que contiene el PID de cada proceso que no realiza operaciones en CPU
// durante el próximo timer-interrupt
void fill_interactive_queue(struct Queue* queue, proc_info_t *procs_info, int procs_count)
{
  for(int i = 0; i < procs_count; i++)
  {
    if(procs_info[i].on_io)
    {
      enqueue(interactive_proc,procs_info[i].pid);
    }else if(in_Queue(interactive_proc, procs_info[i].pid) != -1)
    {
      int pos = in_Queue(interactive_proc, procs_info[i].pid);
      remove_from_Queue(interactive_proc, pos);
    }
  }
}
// Dada una cola de prioridad, determina la cantidad de procesos de dicha cola que no utilizan
// el CPU en el próximo timer-interrupt; basta con verificar si cada elemento de la cola
// se encuentra en la cola que contiene dichos procesos interactivos
int count_interactive_in_queue(struct Queue* queue, struct Queue* interactive_procs)
{
  int count = 0;
  for(int i = 0; i < queue->size; i++)
  {
    if(in_Queue(interactive_procs, queue->array[i]) != -1)
    {
      count++;
    }
  }
  return count;
}
int next_cpu_active_process(struct Queue* queue, struct Queue* queue_time, struct Queue* interactive, int do_action)
{
  int cpu_procs = queue->size - count_interactive_in_queue(queue, interactive);
  int next_pid = -1;
      if(cpu_procs > 0){
        if(in_Queue(interactive, front(queue)) == -1 && !do_action){

          next_pid = front(queue);
          queue_time->array[0] = front(queue_time) + 10; // la constante depende del timer-interrupt-period 
                                                                //  definido en simulation.c                     
        } else if(do_action)
        {
          enqueue(queue, dequeue(queue));
          enqueue(queue_time, dequeue(queue_time));
          // mientras el PID en la cabecera de la cola corresponda a un proceso que no realiza 
          // operaciones en el cpu durante el próximo timer-interrupt, buscar un PID que 
          // realice operaciones en el cpu 
          while(in_Queue(interactive,front(queue)) != -1){
            enqueue(queue, dequeue(queue));
            enqueue(queue_time, dequeue(queue_time));
          }
        next_pid = front(queue);
        // actualiza el tiempo de ejecucion en la cola, del proceso en la cabacera
        queue_time->array[0] = front(queue_time) + 10; 
        }        
      }
  return next_pid;
}
int select_pid(struct Queue* highest_p, struct Queue* second_p, struct Queue* lowest_p, struct Queue* interactive_q,
                   int do_RR) {
    int value = -1;   
    if(is_Empty(highest_p) == 0)
    {
      value = next_cpu_active_process(highest_p,h_priority_time,interactive_q, do_RR);
    } else if(is_Empty(second_p) == 0 && value == -1){
      value = next_cpu_active_process(second_p, s_priority_time, interactive_q, do_RR);
    } else if(is_Empty(lowest_p) == 0 && value == -1){
      value = next_cpu_active_process(lowest_p, l_priority_time, interactive_q, do_RR);
    }
    // printf("PID resultante: %d \n", value);
    return value;
}
/////////////////////////////////////////////////////////////////////////////

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // el valor de accu_time_mlfq se inicializa en -1, si tiene este valor significa
  // que es el primer timer-interrupt en que se tiene un proceso activo desde que 
  // comenzó la simulació
  if(accu_time_mlfq < 0)
  {
    // las tres colas estan vacias, pero hay al menos un proceso activo nuevo
    // agregar el PID de cada proceso activo en la cola de mayor prioridad
    fill_highest_priority(highest_priority,procs_info,procs_count);     
  }  
  

  // Si el proceso actual terminó su ejecución, eliminar el PID del proceso en la cola correspondiente
  if(curr_pid == -1){
    if(in_Queue(highest_priority, previous_pid) == 0)
    {
      dequeue(highest_priority);
      dequeue(h_priority_time);
    } else if(in_Queue(second_priority, previous_pid) == 0){
      dequeue(second_priority);
      dequeue(s_priority_time);
    }else if(in_Queue(lowest_priority, previous_pid) == 0){
      dequeue(lowest_priority);
      dequeue(l_priority_time);
    }
  }  
  
  // Si han llegado procesos nuevos, insertar el PID de cada uno en la cola de mayor prioridad
  fill_highest_priority(highest_priority,procs_info,procs_count);
  // Actualizar el estado de cada cola, teniendo en cuenta el tiempo 
  // de ejecución de cada proceso activo en su respectiva cola
  
  update_queues(procs_info, procs_count);  
  // Si se ha consumido un tiempo igual a un múltiplo del valor PRIORITY_BUS_PERIOD
  // mover el PID de cada proceso ea la cola de mayor prioridad  
  if(accu_time_mlfq > 0 && curr_time % PRIORITY_BUS_PERIOD == 0)
  {
    priority_update();
  } 
 
  // Determinar dentro de los procesos activos, los que no utilizan CPU en el próximo timer-interrupt
  fill_interactive_queue(interactive_proc, procs_info, procs_count);
  // Seleccionar el PID del proceso a ejecutar     
  int round_robin =  accu_time_mlfq > 0 && curr_time % TIME_SLICE == 0;  
  int next_pid = select_pid(highest_priority,second_priority,lowest_priority,interactive_proc, round_robin);  
 
  accu_time_mlfq = curr_time;
  previous_pid =  next_pid;
 
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
  // cola para almacenar el PID de cada proceso que se encuentre realizando una operación fuera del CPU
  // en el próximo timer-interrupt
  interactive_proc = Create_Queue(10000);

  PRIORITY_BUS_PERIOD = 100;
  // múltiplo del TIMER_INTERRUPT_PERIOD, se actualiza en cada timer_interrupt
  accu_time_mlfq = -1;
  // guarda el PID del proceso que se ejecutó en el timer_interrupt anterior
  previous_pid = -1;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
