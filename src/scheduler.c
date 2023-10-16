#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

#define SLICE_TIME 60

#define RR_SLICE_TIME 40

#define PRIORITY_BOOST 300

static ml_queue_t *l_queue;

static process_queue_t new_queue() {
  process_queue_t queue = {
    (proc_info_t *)malloc(MAX_PROCESS_COUNT * (sizeof (proc_info_t))),
    0
  };

  for (int i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    queue.processes[i].executed_time = 0;
    queue.processes[i].on_io = 0;
    queue.processes[i].pid = 0;
  }
  

  return queue;
}

static ml_queue_t new_ml_queue(int levels) {
  
  ml_queue_t ml_queue = {
    (process_queue_t *)malloc(levels * (sizeof (process_queue_t))),
    (tuple_int_t *)malloc(levels * (sizeof (tuple_int_t))),
    0,
    levels
  };

  for(int i = 0; i<levels; i++){
    ml_queue.process_queue[i] = new_queue();
    tuple_int_t temp = {0, 0};
    ml_queue.pid_proc[i] = temp;
  }

  return ml_queue;
}

// Mezcla dos listas de PID de procesos
static int merge(tuple_int_t *left, int l_count, tuple_int_t *right, int r_count) {
  tuple_int_t *new_arr = malloc( (l_count+r_count)* sizeof(tuple_int_t));
  
  if(l_count == 0){
    return r_count;
  }
  else if (r_count == 0)
  {
    right = left;
    return l_count;
  }
  
  printf("3444 \n");

  int l = 0; 
  int r = 0;
  int idx = 0;

  while (l< l_count && r< r_count) {
    if (left[l].right < right[r].right) {
      new_arr[idx].left = left[l].left;
      new_arr[idx].right = left[l].right;

      idx ++;
      l ++;

    }
    else {
      new_arr[idx].left = right[r].left;
      new_arr[idx].right = right[r].right;

      idx ++; 
      r ++;
    }
  }

  if (l == l_count) {
    while (r < r_count) {
      new_arr[idx].left = right[r].left;
      new_arr[idx].right = right[r].right;

      r ++;
      idx ++;
    }
  }

  if (r == r_count) {
    while (l < l_count) {
      new_arr[idx].left = left[l].left;
      new_arr[idx].right = left[l].right;

      l ++;
      idx ++;
    }
  }
  int new_count = l_count+r_count;
  tuple_int_t ** pointer = &new_arr;
  right = *pointer;
  
  return new_count;
}

// Agrega un proc_info_t a la cola
static void push(proc_info_t process, process_queue_t queue) {

  queue.processes[queue.count].on_io = process.on_io;
  queue.processes[queue.count].pid = process.pid;
  queue.processes[queue.count].executed_time = process.executed_time;
}

// Agrega el PID de un proceso a una cola
static void push_p(tuple_int_t *arr, int count, int num, int level){
  arr[count].left = level;
  arr[count].right = num;
}

static int binary_search(process_queue_t queue, int pid) {

  int st = 0;
  int end = queue.count - 1;

  while (st <= end) {
    int mid = (st+end)/2;
    if (pid < queue.processes[mid].pid) {
      end = mid - 1;
    }
    else if (pid > queue.processes[mid].pid)
    {
      st = mid + 1;
    }
    else return mid;
  }

  return -1;
}

static int binary_search_p(tuple_int_t *arr, int count, int num) {

  int st = 0;
  int end = count - 1;

  while (st <= end) {
    int mid = (st+end)/2;
    if (num < arr[mid].right) {
      end = mid - 1;
    }
    else if (num > arr[mid].right) {
      st = mid + 1;
    }
    else return mid;
  }

  return -1;
}

// Elimina un proceso de la cola
static proc_info_t sup(process_queue_t queue, int pid) {

  int i = binary_search(queue, pid);
  
  proc_info_t process = queue.processes[i];

  for(int j = i+1; j< queue.count; j++) {

    queue.processes[j-1].executed_time = queue.processes[j].executed_time;
    queue.processes[j-1].pid = queue.processes[j].pid;
    queue.processes[j-1].on_io = queue.processes[j].on_io;

  }
  
  return process;
}

static void sup_p(tuple_int_t *arr, int count, int pid) {

  int i = binary_search_p(arr, count, pid);
  for(int j = i+1; j< count; j++) {

    arr[j-1].left = arr[j].left;
    arr[j-1].right = arr[j].right;

  }
}

static void sup_all(tuple_int_t *process, int count) {

  for (int i = 0; i < count; i++)
  {
    sup_p(l_queue->pid_proc, count, process[i].right);
    l_queue->process_count -=1;
    sup(l_queue->process_queue[process[i].left], process[i].right);
    l_queue->process_queue[process[i].left].count --;
  }
}

static void level_down(tuple_int_t *process, int count) {

  for (int i = 0; i < count; i++)
  {
    if (process[i].left < l_queue->count-1)
    {
      proc_info_t temp = sup(l_queue->process_queue[process[i].left], process[i].right);
      int idx = binary_search_p(l_queue->pid_proc, l_queue->process_count, process[i].right);
      l_queue->pid_proc[idx].left += 1;
      push(temp, l_queue->process_queue[l_queue->pid_proc[idx].left]);
    }
    
  }
  
}

static void level_up(tuple_int_t* process, int count) {

  for (int i = 0; i < count; i++)
  {
    if (process[i].left < l_queue->count-1)
    {
      proc_info_t temp = sup(l_queue->process_queue[process[i].left], process[i].right);
      int idx = binary_search_p(l_queue->pid_proc, l_queue->process_count, process[i].right);
      l_queue->pid_proc[idx].left += 1;
      push(temp, l_queue->process_queue[l_queue->pid_proc[idx].left]);
    }
    
  }
}


// Comprueba si hay procesos nuevos y los agrega a la cola
// Actualiza la prioridad de los procesos segun su comportamiento
static void update_priority(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {

  int old = 0;
  int new = 0;

  tuple_int_t *temp = malloc(MAX_PROCESS_COUNT* sizeof(tuple_int_t));
  int temp_count = 0;


  while(new < procs_count && old < l_queue->process_count) {
    if(procs_info[new].pid == l_queue->pid_proc[old].right) {
      old ++;
      new ++;
    }
    else if (procs_info[new].pid > l_queue->pid_proc[old].right) {
      push_p(temp, temp_count, procs_info[new].pid, 0);
      temp_count ++;

      push(procs_info[new], l_queue->process_queue[0]);
      l_queue->process_queue[0].count ++;
      new ++;
    }
  }


  if(old == l_queue->process_count) {
    while (new < procs_count) {
      push_p(temp, temp_count, procs_info[new].pid, 0);
      temp_count += 1;


      push(procs_info[new], l_queue->process_queue[0]);
      l_queue->process_queue[0].count ++;
      new ++;
    }
  }

  l_queue->count = merge(temp, temp_count, l_queue->pid_proc, l_queue->count);
  free(temp);


  tuple_int_t *sup = malloc(MAX_PROCESS_COUNT* sizeof(tuple_int_t));
  int sup_count = 0;

  tuple_int_t *up = (tuple_int_t *)malloc(MAX_PROCESS_COUNT* sizeof(tuple_int_t));
  int up_count = 0;

  tuple_int_t *down = (tuple_int_t *)malloc(MAX_PROCESS_COUNT* sizeof(tuple_int_t));
  int down_count = 0;

  for(int i = 0; i< l_queue->count; i++) {
    for(int j = 0; j< l_queue->process_queue[i].count; j++) {
      if (process_total_time(l_queue->process_queue[i].processes[j].pid) == 0) {
        push_p(sup, sup_count, l_queue->process_queue[i].processes[j].pid, i);
        sup_count += 1;
      }
      else if (l_queue->process_queue[i].processes[j].executed_time >= SLICE_TIME) {
        push_p(down, down_count, l_queue->process_queue[i].processes[j].pid, i);
        down_count += 1;
      }
      else if (curr_time%PRIORITY_BOOST == 0)
      {
        push_p(up, up_count, l_queue->process_queue[i].processes[j].pid, i);
        up_count += 1;
      }
      
    }
  }
  sup_all(sup, sup_count);
  level_up(up, up_count);
  level_down(down, down_count);
  free(up);
  free(down);
}

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

  int ld = __INT_MAX__;
  int pid = curr_pid;

  if(process_total_time(pid) == 0){

    for(int i = 0; i < procs_count; i++){

      int proc_pid = procs_info[i].pid;
      int duration = process_total_time(proc_pid);
      
      if(duration < ld){
        pid = proc_pid;
        ld = duration;
      }
    }
  }
  
  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int st = __INT_MAX__;
  int pid = curr_pid;

  for(int i = 0; i < procs_count; i++){

    int proc_pid = procs_info[i].pid;
    int t_exc = process_total_time(proc_pid) - procs_info[i].executed_time;
    
    if(t_exc < st){
      pid = proc_pid;
      st = t_exc;
    }
  }
  
  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int pid = curr_pid;
  int IO = 0;
  for(int i = 0; i< procs_count; i++){
      if(procs_info[i].pid == curr_pid){

        IO = procs_info[i].on_io;
      }
    }
  if(curr_time%RR_SLICE_TIME == 0 || IO == 1){

    for(int i = 0; i< procs_count; i++){
      if(procs_info[i].pid == curr_pid){
        int x = 1;
        pid = procs_info[(i+x)%procs_count].pid;
        while (x < procs_count && procs_info[(i+x)%procs_count].on_io == 1)
        {
          pid = procs_info[(i+x)%procs_count].pid;
          x ++;
        }
      }
    }

  }
  if(-1 == pid) return procs_info[0].pid;

  return pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

  if(curr_time%SLICE_TIME == 0){
    update_priority(procs_info, procs_count, curr_time, curr_pid);
    
    int level = 0;
    int count = 0;

    while(level < l_queue->count) {

      count = l_queue->process_queue[level].count;
      if (count != 0){
        break;
      }
      level += 1;
    }
    printf("%d eso \n", level);

    return rr_scheduler(l_queue->process_queue[level].processes, l_queue->process_queue[level].count, curr_time, curr_pid);
  }

  return curr_pid;
}


// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  ml_queue_t queue = new_ml_queue(3);
  l_queue = &queue;

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
