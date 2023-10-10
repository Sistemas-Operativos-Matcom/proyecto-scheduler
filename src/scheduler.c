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

static bool contains_pid(proc_info_t* procs_info, int procs_count, int pid) {
  for (int i = 0; i < procs_count; i++)
    if (procs_info[i].pid == pid)return true;
  return false; 
}

static void delete_completed_procs(queue_t* q, proc_info_t* procs_info, int procs_count) {
  for (int i = 0; i < q->len; i++) {
    proc_info_t t = pop_item(q);
    
    if (contains_pid(procs_info, procs_count, t.pid))push_item(q, t);
  }
}

static void concat_queues(queue_t* to, queue_t** from, int count) {
  for (int j = 0; j < count; j++) {
    queue_t* _from = from[j];

    if (!_from->len)
      continue;

    else if (!to->len) {
      to->head = _from->head;
      to->tail = _from->tail;
      to->len = _from->len;
    }

    else {
      _from->tail->next = to->head;
      to->head = _from->head;

      to->len += _from->len;
    }

    start_queue(_from);
  }
}

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

  //Add new procs
  bool is_proc_ended = false;
  for (int i = last_len < 2 ? 0 : last_len - 2; i < procs_count; i++) {
    if (l1 == procs_info[i].pid || l2 == procs_info[i].pid){
      is_proc_ended = true;
      continue;
    }

    push_item(&prior_high, procs_info[i]);
    prior_high.tail->value.executed_time = 0;
  }

  last_len = procs_count;
  l1 = procs_count - 1 >= 0 ? procs_info[procs_count - 1].pid : -1;
  l2 = procs_count - 2 >= 0 ? procs_info[procs_count - 2].pid : -1; 


  //Update priority of old procs
  if (update_time == update_count) {
    update_count = 0;

    queue_t* qq[] = {&prior_low, &prior_mid};
    concat_queues(&prior_high, qq, 2);
  
  } update_count++;

  //Decrease priority if it reached the time_slice
  if (prior_high.len && prior_high.head->value.executed_time == time_slice) {
    proc_info_t tmp = pop_item(&prior_high);
    tmp.executed_time = 0;

    push_item(&prior_mid, tmp);
  }
  if (prior_mid.len && prior_mid.head->value.executed_time == time_slice) {
    proc_info_t tmp = pop_item(&prior_mid);
    tmp.executed_time = 0;

    push_item(&prior_low, tmp);
  }

  //Delete completed procs
  if (is_proc_ended) {
    delete_completed_procs(&prior_high, procs_info, procs_count);
    delete_completed_procs(&prior_mid , procs_info, procs_count);
    delete_completed_procs(&prior_low , procs_info, procs_count);
  }

  //Round-robin if the same priority, else go to lower priority
  queue_t* rr = NULL;
  if (prior_high.len)     rr = &prior_high;
  else if (prior_mid.len) rr = &prior_mid;
  else if (prior_low.len) rr = &prior_low;

  if (rr != NULL && rr->len) {
    rr->head->value.executed_time++;

    if (rr->head->value.executed_time && rr->head->value.executed_time % rr_time == 0) {
      proc_info_t tmp = pop_item(rr);
      push_item(rr, tmp);
    }
    return rr->head->value.pid;
  }
  
  return -1;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "rr")   == 0) return *rr_scheduler;
  if (strcmp(name, "sfj")   == 0) return *sfj_scheduler;
  if (strcmp(name, "stcf")   == 0) return *stcf_scheduler;
  if (strcmp(name, "mlfq")   == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
