#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "simulation.h"
#include "queue.h"

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

//Declare utility functions
static bool contains_pid(proc_info_t* procs_info, int procs_count, int pid);
static void delete_completed_procs(queue_t* q, proc_info_t* procs_info, int procs_count);
static void concat_queues(queue_t* to, queue_t** from, int count);
static bool is_on_io(int pid, proc_info_t* procs, int procs_count);
static int get_executable_pid(queue_t* procs, proc_info_t* procs_info, int procs_count, int rr_time);


int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sfj_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  int pid = procs_info[0].pid, min_time = process_total_time(procs_info[0].pid), tmp;
  
  for (int i = 0; i < procs_count; i++) {
    if (curr_pid == procs_info[i].pid) return curr_pid;
    
    tmp = process_total_time(procs_info[i].pid);
    if (tmp < min_time  && !procs_info[i].on_io) {
      tmp = min_time;
      pid = procs_info[i].pid;
    }
  }
  return pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  int pid = procs_info[0].pid, min_time = process_total_time(procs_info[0].pid) - procs_info[0].executed_time, tmp;

  for (int i = 0; i < procs_count; i++) {
    tmp = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    if (tmp < min_time && !procs_info[i].on_io) {
      tmp = min_time;
      pid = procs_info[i].pid;
    }
  }

  return pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  const  int timeskip = 5;
  static int interrupts_elapsed = 0;
  static int curr_pos = 0;
    
  if (timeskip > ++interrupts_elapsed) {
    curr_pos = curr_pos % procs_count;
  }
  else {
    interrupts_elapsed = 0;
    curr_pos = (curr_pos + 1) % procs_count;
  }
  
  return procs_info[curr_pos].pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  //Declare variables, they are static as do not 
  //need to redeclare each function call
  static bool is_init = false;
  static queue_t prior_high, prior_mid,prior_low;
  
  static int last_len = -1, l1 = -1, l2 = -1;
  static int update_count = 0;

  const int rr_time = 3;
  const int time_slice = 12;
  const int update_time = 60;
  
  //Initialize queues
  if (!is_init) {
    start_queue(&prior_high);
    start_queue(&prior_mid);
    start_queue(&prior_low);

    is_init = true;
  }

  //Add new procs to the queue
  //as only 2 procs can leave the queue at the same time, we check 
  //from there and add the newcomers
  for (int i = last_len < 2 ? 0 : last_len - 2; i < procs_count; i++) {
    if (l1 == procs_info[i].pid || l2 == procs_info[i].pid){
      continue;
    }

    push_item(&prior_high, procs_info[i]);
    prior_high.tail->value.executed_time = 0; //the executed_time is used to store the time a procces hs been in cpu, initially 0
  }

  last_len = procs_count;
  l1 = procs_count - 1 >= 0 ? procs_info[procs_count - 1].pid : -1;
  l2 = procs_count - 2 >= 0 ? procs_info[procs_count - 2].pid : -1; 

  //Delete completed procs
  delete_completed_procs(&prior_high, procs_info, procs_count);
  delete_completed_procs(&prior_mid , procs_info, procs_count);
  delete_completed_procs(&prior_low , procs_info, procs_count);

  // Update priority of old procs
  if (update_time == update_count) {
    update_count = 0;

    queue_t* qq[] = {&prior_low, &prior_mid};
    concat_queues(&prior_high, qq, 2);
  
  } update_count++;

  //Decrease priority if it reached the time_slice
  if (prior_mid.len && prior_mid.head->value.executed_time == time_slice) {
    proc_info_t tmp = pop_item(&prior_mid);
    tmp.executed_time = 0;

    push_item(&prior_low, tmp);
  }
  if (prior_high.len && prior_high.head->value.executed_time == time_slice) {
    proc_info_t tmp = pop_item(&prior_high);
    tmp.executed_time = 0;

    push_item(&prior_mid, tmp);
  }

  //Round-robin if the same priority, else go to lower priority
  //Ignore the procs performing IO operations
  //If nothing can run, return -1
  int ans = get_executable_pid(&prior_high, procs_info, procs_count, rr_time);
  if (ans == -1) ans = get_executable_pid(&prior_mid, procs_info, procs_count, rr_time);
  if (ans == -1) ans = get_executable_pid(&prior_low, procs_info, procs_count, rr_time);

  return ans;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "rr")   == 0) return *rr_scheduler;
  if (strcmp(name, "sjf")   == 0) return *sfj_scheduler;
  if (strcmp(name, "stcf")   == 0) return *stcf_scheduler;
  if (strcmp(name, "mlfq")   == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}

/* Helper functions */

static bool contains_pid(proc_info_t* procs_info, int procs_count, int pid) {
  /** 
   * returns true if the given proc_info_t array contains
   * the given pid, else, it returns false 
  */

  for (int i = 0; i < procs_count; i++)
    if (procs_info[i].pid == pid)return true;
  return false; 
}

static void delete_completed_procs(queue_t* q, proc_info_t* procs_info, int procs_count) {
  /**
   * it takes a queue of proc_info_t items and removes the ones that are
   * not in the procs_info array
   * the approach here is O(|q| * |procs_info|), which is bad, but as performance here is 
   * not the main goal I decided to improve the code readability by doing this
  */
  
  int n = q->len;
  for (int i = 0; i < n; i++) {
    proc_info_t t = pop_item(q);

    if (contains_pid(procs_info, procs_count, t.pid))push_item(q, t);
  }
}

static void concat_queues(queue_t* to, queue_t** from, int count) {
  /**
   * it takes a queue and an array of queues, then iterates for each element of the 
   * the array of queues and pushes all of them in the original
   * resulting in a queue that contains all the elements of the others
  */
  
  for (int j = 0; j < count; j++) {
    queue_t* _from = from[j];

    while (_from->len) {
      proc_info_t proc = pop_item(_from);
      proc.executed_time = 0;

      push_item(to, proc);
    }

    start_queue(_from);
  }
}

static bool is_on_io(int pid, proc_info_t* procs, int procs_count) {
  /**
   * checks if a given pid is on IO operations
  */
  for (int i = 0; i < procs_count; i++) {
    if (procs[i].pid == pid)return procs[i].on_io != 0;
  }
  return false;
}

static int get_executable_pid(queue_t* procs, proc_info_t* procs_info, int procs_count, int rr_time) {
  /**
   * given a queue of pid's, it returns the first procces which is
   * not performing IO operations, then updates it's executed time
   * 
   * if no procces can run, then returns -1
  */
 
  for (int i = 0; i < procs->len && (is_on_io(procs->head->value.pid, procs_info, procs_count) || (procs->head->value.executed_time % rr_time == 0 && i == 0)); i++) {
    proc_info_t proc = pop_item(procs);
    push_item(procs, proc);
  }

  if (procs->len && !is_on_io(procs->head->value.pid, procs_info, procs_count)) {
    procs->head->value.executed_time++;
    return procs->head->value.pid;
  }
  
  return -1;
}