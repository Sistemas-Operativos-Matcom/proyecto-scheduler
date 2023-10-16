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
    malloc(MAX_PROCESS_COUNT * sizeof (int)),
    0
  };

  for (int i = 0; i < MAX_PROCESS_COUNT; i++)
  {
    queue.processes[i] = 0;
  }
  

  return queue;
}

static ml_queue_t new_ml_queue(int levels) {
  
  ml_queue_t ml_queue = {
    malloc(levels * (sizeof (process_queue_t))),
    malloc(levels * (sizeof (tuple_int_t))),
    0,
    levels
  };
    printf("pid list dir created at %p \n", ml_queue.pid_proc);

  return ml_queue;
}

// Agrega un proc_info_t a la cola
static void push(int process, process_queue_t *queue) {

  queue->processes[queue->count] = process;
  int count = queue->count;
  for (int i = count; i > 1 ; i--)
  {
    if(queue->processes[i-1] > queue->processes[i]) {
      int temp = queue->processes[i-1];
      queue->processes[i-1] = queue->processes[i];
      queue->processes[i] = temp;
    }
  }
  
}

// Agrega el PID de un proceso a una cola
static void push_p(tuple_int_t *arr, int count, int pid, int level){
  arr[count].left = level;
  arr[count].right = pid;
}

static int binary_search(process_queue_t queue, int pid) {

  int st = 0;
  int end = queue.count - 1;

  while (st <= end) {
    int mid = (st+end)/2;
    if (pid < queue.processes[mid]) {
      end = mid - 1;
    }
    else if (pid > queue.processes[mid])
    {
      st = mid + 1;
    }
    else return mid;
  }

  return -1;
}

static int binary_search_p(tuple_int_t* arr, int count, int pid) {

  int st = 0;
  int end = count - 1;

  while (st <= end) {
    int mid = (st+end)/2;
    if (pid < arr[mid].right) {
      end = mid - 1;
    }
    else if (pid > arr[mid].right)
    {
      st = mid + 1;
    }
    else return mid;
  }

  return -1;
}


// Elimina un proceso de la cola
static int sup(process_queue_t queue, int pid) {

  int i = binary_search(queue, pid);
  
  int process = queue.processes[i];

  for(int j = i+1; j< queue.count; j++) {
    queue.processes[j-1] = queue.processes[j];
  }
  
  return process;
}


static void sup_all(int *process, int count) {

  for (int i = 0; i < count; i++)
  {
    int idx = binary_search_p(l_queue->pid_proc, l_queue->count, process[i]);
    l_queue->pid_proc[idx].left = 0;
    l_queue->process_count --;
    int lvl = l_queue->pid_proc[idx].right;

    sup(l_queue->process_queue[lvl], process[i]);
    l_queue->process_queue[lvl].count --;
  }
}

static void level_down(int *process, int count) {

  for (int i = 0; i < count; i++)
  {
    int dir = binary_search_p(l_queue->pid_proc, l_queue->count, process[i]);

    if (l_queue->pid_proc[dir].right < l_queue->count-1)
    {
      int lvl = l_queue->pid_proc[dir].right;
      sup(l_queue->process_queue[lvl], process[i]);
      l_queue->pid_proc[dir].right += 1;
      int idx = l_queue->pid_proc[dir].right;
      push(process[i], &l_queue->process_queue[idx]);
    }
    
  }
  
}

static void level_up(int* process, int count) {

  for (int i = 0; i < count; i++)
  {
    int dir = binary_search_p(l_queue->pid_proc, l_queue->count, process[i]);

    if (l_queue->pid_proc[process[i]].right != 0)
    {
      int lvl = l_queue->pid_proc[dir].right;
      sup(l_queue->process_queue[lvl], process[i]);
      l_queue->pid_proc[dir].right = 0;
      push(process[i], &l_queue->process_queue[0]);
    }
    
  }
}


// Comprueba si hay procesos nuevos y los agrega a la cola
// Actualiza la prioridad de los procesos segun su comportamiento
static void update_priority(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {

  int new = 0;

  while(new < procs_count) {
    printf("processs pid %d \n",procs_info[new].pid);
    printf("pid list dir %p \n",l_queue->pid_proc);

    int idx = binary_search_p(l_queue->pid_proc, l_queue->process_count, procs_info[new].pid);
    if(idx != 1) {
      new ++;
    }
    else {
      push_p(l_queue->pid_proc, l_queue->process_count, procs_info[new].pid, 0);
      l_queue->process_count ++;

      push(procs_info[new].pid, &l_queue->process_queue[0]);
      l_queue->process_queue[0].count ++;
      new ++;
    }
    
  }


  int *sup = malloc(MAX_PROCESS_COUNT* sizeof(int));
  int sup_count = 0;

  int *up = malloc(MAX_PROCESS_COUNT* sizeof(int));
  int up_count = 0;

  int *down = malloc(MAX_PROCESS_COUNT* sizeof(int));
  int down_count = 0;

  // Eliminar procesos inactivos

  for(int i = 0; i< l_queue->count; i++) {
    for(int j = 0; j< l_queue->process_queue[i].count; j++) {
      if (process_total_time(l_queue->process_queue[i].processes[j]) == 0) {
        sup[sup_count] = l_queue->process_queue[i].processes[j];
        sup_count += 1;
      }
    }
  }

  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].executed_time >= SLICE_TIME) {
      down[down_count] = procs_info[i].pid;
      down_count += 1;
    }
    if (curr_time%PRIORITY_BOOST == 0)
    {
      up[down_count] = procs_info[i].pid;
      up_count += 1;
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

int int_rr_scheduler(int *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int pid = curr_pid;
  if(curr_time%RR_SLICE_TIME == 0){

    for(int i = 0; i< procs_count; i++){
      if(procs_info[i] == curr_pid){
        pid = procs_info[(i+1)%procs_count];
      }
    }

  }
  if(-1 == pid) return procs_info[0];

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

    return int_rr_scheduler(l_queue->process_queue[level].processes, l_queue->process_queue[level].count, curr_time, curr_pid);
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
