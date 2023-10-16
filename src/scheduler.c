#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include <limits.h>
#include "queue.h"

queue_t *queue_1;
queue_t *queue_2;
queue_t *queue_3;



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
    int temp = INT_MAX;
    int new_curr_pid = curr_pid;
    if (curr_pid != -1)
    {
      return curr_pid;
    }
    for (int i = 0; i < procs_count; i++) {
      if (process_total_time(procs_info[i].pid) < temp) {
        new_curr_pid = procs_info[i].pid;
        temp = process_total_time(procs_info[i].pid);
      }
    }

  return new_curr_pid;

  
}
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int new_curr_pid = curr_pid;
  int min_remaining_time = INT_MAX;
  
  for (int i = 0; i < procs_count; i++) {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < min_remaining_time) {
      min_remaining_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      new_curr_pid = procs_info[i].pid;
    }
  }

  return new_curr_pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
    



    static int last_index = 0;
    int next_pid = -1;

    if (curr_pid == -1) {
      next_pid = procs_info[last_index].pid;    
    }
    else if(last_index + 1 == procs_count) {
      next_pid = procs_info[0].pid;
      last_index = 0;
    }
    else {
      next_pid = procs_info[last_index + 1].pid;
      last_index += 1 ;
    }
    
   
    return next_pid;
    
}


/*

queue_t *queues[3];

void init_queues() {
  queues[0] = queue_1;
  queues[1] = queue_2;
  queues[2] = queue_3;
}



int find_next_pid(int pid) {
    node_t *curr_node = NULL;
    node_t *prev_node = NULL;
    int index = 0;
    for (int i = 0; i < 3; i++) {
      curr_node = queues[i]->head;
      while (curr_node != NULL) {
        if (curr_node->pid == pid) {
            prev_node = curr_node;
            curr_node = curr_node->next;
            if(pid == -1) free(prev_node);
            if(curr_node == NULL){
              curr_node = queues[i]->head;
              
            }
            index = i;
            break;
            
        }
      }
        
        
    
        
    }
    
    for (int j = 0; j < index; j++)
    {
      if(!is_empty(queues[j])){
        curr_node = queues[j]->head;
        return curr_node->pid;
        break;
      }
    }

    if(index != 0) return curr_node->pid;

    for(int k = 0; k < 3; k++){
      if(!is_empty(queues[k])){
        curr_node = queues[k]->head;
        return curr_node->pid;
        break;
      }
    }

    
   
    return -1; // pid not found in queue
}

int find_pid_in_queue(queue_t *queue, int pid) {
    node_t *current_node = queue->head;
    while (current_node != NULL) {
        if (current_node->pid == pid) {
            return 1; // pid found in queue
        }
        current_node = current_node->next;
    }
    return 0; // pid not found in queue
}

//verificar si el proceso actual estuvo todo el time slice en el cpu
int is_process_in_cpu(proc_info_t *procs_info, int procs_count, int curr_pid) {
  
  proc_info_t curr_proc;

  for(int i = 0; i < procs_count; i++){
    if(procs_info[i].pid == curr_pid){
      curr_proc =  procs_info[i];
      break;
    }
    
  }

  curr_proc.executed_time -= 10;
  for (int j = 1; j <= 10; j++) {
    if(curr_proc.on_io) return 0;
    curr_proc.executed_time += 1;
    if(curr_proc.executed_time = process_total_time(curr_pid)) break; 

  }

  return 1;
}



int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
      static int priority_boost = 0;
      int queue = 0;
      int first = 1;

      for (int j = 0; j < procs_count; j++){
        if(procs_info[j].executed_time == 0 && !find_pid_in_queue(queue_1, procs_info[j].pid))
        {
          enqueue (queue_1, procs_info[j].pid);
        }

      }

      for (int i = 0; i < 3; i++){
        queue_t *curr_queue = queues[i];
        if (find_pid_in_queue(curr_queue, curr_pid)){
          if(is_process_in_cpu(procs_info, procs_count, curr_pid)){
            if(i != 2){
              node_t *temp = dequeue(curr_queue, curr_pid);
              enqueue(queues[i+1],temp); 

            }
            

          }
          break;
        }
      }

      

      int pid = find_next_pid(curr_pid);

      priority_boost++;

      if(priority_boost == 3)
      {
        for(int k = 1; k < 3; k++)
        {
          queue_t *curr_queue = queues[k];
          node_t *current_node = queues[k]->head;
          while (current_node != NULL) {
            node_t *temp = dequeue(curr_queue, current_node->pid);
            enqueue(queues[0],temp);
            current_node = current_node->next;
          }
          
        }
        priority_boost = 0;
        pid = queues[0]->head->pid;
      }

      return pid;
      
      
      
     

  


        
  




}

*/

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  /*if (strcmp(name, "mlfq") == 0) {
    enqueue(queue_1, -1);
    enqueue(queue_2, -1);
    enqueue(queue_3, -1);
    return *mlfq_scheduler;
  }*/


  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
