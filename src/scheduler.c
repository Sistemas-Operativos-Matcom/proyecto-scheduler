#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "queue.h"

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
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  int n = procs_info[0].pid;
  
  for(int i = 1; i < procs_count; i++){
    int m = procs_info[i].pid;
    
    if(m == curr_pid) return curr_pid;
    if(process_total_time(n) > process_total_time(m)) n = m;
  }

  return n;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  int n = procs_info[0].pid;
  
  for(int i = 1; i < procs_count; i++){
    int m = procs_info[i].pid;
    
    if(process_total_time(n) - procs_info[n].executed_time > process_total_time(m) - procs_info[m].executed_time)
     n = m;
  }

  return n;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  static int rr_pointer = -1;

  if(rr_pointer + 1 >= procs_count){
    rr_pointer = 0;
  } 

  else rr_pointer = rr_pointer + 1; 

  return procs_info[rr_pointer].pid;  
}

void queue_work(queue_t* q, proc_info_t *procs_info, int procs_count){
    bool fint = false;
    int n = q->len;

    for(int i = 0; i < n; i++){
      if(q->nxt == q->last) q->nxt = q->first;
    
      for(int j = 0; j < procs_count; j++){
        if(procs_info[j].pid == q->nxt->value.pid){
          q->nxt->value.on_io = procs_info[i].on_io;
          q->nxt->value.executed_time = procs_info[i].executed_time;
          fint = true;
          break;
        }
      }

      if(!fint) remove_queue(q, q->nxt->value.pid); 

      else q->nxt = q->nxt->next;
    }  

  q->nxt = q->nxt->next;
}

proc_info_t* find_next_procs_exce(queue_t* q){
  node_t* n = q->nxt;
  bool ok = false;

  for(int i = 0; i < q->len; i++){
    if(n == q->last) n = q->first;

    if(n->value.on_io == 0) {
        n = q->nxt;
        q->nxt = n->next;
        break;
      } 
  }

  return &(n->value); 
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  static bool start = false;
  static queue_t* one;
  static queue_t* two;
  static queue_t* three;

  const int S = 3;
  static int count_S = 0;

  static int last_total_count = 0;
  proc_info_t* node_ret;

  if(!start){
    start_queue(one);
    start_queue(two);
    start_queue(three);

    start = true;
  }

  queue_work(one, procs_info, procs_count);
  queue_work(two, procs_info, procs_count);
  queue_work(three, procs_info, procs_count);

  if(last_total_count <= procs_count){
    proc_info_t p = procs_info[last_total_count - 1];
    int i = last_total_count;
    
    if(!find_queue(one, p) && !find_queue(two, p) && !find_queue(three, p)) i--;

    while(i < procs_count) {
      append_queue(one, procs_info[i]);
      i++;
    }
  }

  last_total_count = procs_count;

  node_ret = find_next_procs_exce(one);

  if(node_ret != NULL){
    remove_queue(one, node_ret->pid);
    append_queue(two, *node_ret);
  }

  else node_ret = find_next_procs_exce(two);

  if(node_ret != NULL){
    remove_queue(two, node_ret->pid);
    append_queue(three, *node_ret);
  }

  else node_ret = find_next_procs_exce(three);

  if(node_ret != NULL) return procs_info[0].pid;

  if(count_S + 1 == S){
    count_S = 0;

    for(int i = 0; i < two->len; i++){
      if(two->nxt == two->last) two->nxt = two->first;

      append_queue(one, two->nxt->value);
      remove_queue(two, two->nxt->value.pid);
    }

    two->nxt = NULL;

    for(int i = 0; i < three->len; i++){
      if(three->nxt == three->last) three->nxt = three->first;

      append_queue(one, three->nxt->value);
      remove_queue(three, three->nxt->value.pid);
    }

    three->nxt = NULL;
  }
    
  return node_ret->pid;
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

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
