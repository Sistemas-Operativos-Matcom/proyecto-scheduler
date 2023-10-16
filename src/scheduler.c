#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "queue.h"

#define NUM_QUEUES 3

queue_t queues[NUM_QUEUES];

void init_queue(int qid, int capacity) {
  queues[qid].procs = (proc_info_t *)malloc(capacity * sizeof(proc_info_t));
  queues[qid].executed_time = (int *)malloc(capacity * sizeof(int));
  queues[qid].count = 0;
  queues[qid].capacity = capacity;
}

void enqueue(int qid, proc_info_t proc, int time) {
  if (queues[qid].count == queues[qid].capacity) {
    queues[qid].capacity *= 2;
    queues[qid].procs = (proc_info_t *)realloc(queues[qid].procs, queues[qid].capacity * sizeof(proc_info_t));
    queues[qid].executed_time = (int *)realloc(queues[qid].executed_time, queues[qid].capacity * sizeof(int));
  }

  queues[qid].executed_time[queues[qid].count] = time;
  queues[qid].procs[queues[qid].count] = proc;
  queues[qid].count++;
}

proc_info_t dequeue(int qid) {
  proc_info_t proc = queues[qid].procs[0];

  for (int i = 0; i < queues[qid].count - 1; i++) {
    queues[qid].procs[i] = queues[qid].procs[i + 1];
    queues[qid].executed_time[i] = queues[qid].executed_time[i + 1];

  }
  queues[qid].count--;

  return proc;
}

int update_queues(proc_info_t *procs_info, int procs_count, int last_max_pid) {
    int time, count;

    for(int qid = 0; qid < NUM_QUEUES; qid++) {
        count = queues[qid].count;

        for(int i = 0; i < count; i++) {
            time = queues[qid].executed_time[0];
            proc_info_t proc = dequeue(qid);

            for(int j = 0; j < procs_count; j++) {
                if(procs_info[j].pid == proc.pid) {
                    enqueue(qid, proc, time);
                    break;
                }
            }
        }
    }

    int max_pid = -1;
    for(int i = 0; i < procs_count; i++) {
        if(procs_info[i].pid > last_max_pid) {
            enqueue(0, procs_info[i], 0);

            if(procs_info[i].pid > max_pid) {
                max_pid = procs_info[i].pid;
            }
        }
    }

    return max_pid != -1 ? max_pid : last_max_pid;
}


int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

  static int curr_ejec_pid = -1;

  int temp_pid = -1, time = 100000000;

  for(int i = 0; i < procs_count; i++){
    if (curr_ejec_pid == procs_info[i].pid)
      return curr_ejec_pid;
    if(process_total_time(procs_info[i].pid) - procs_info[i].executed_time < time){
      time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      temp_pid = procs_info[i].pid;
    }
  }

  curr_ejec_pid = temp_pid;

  return temp_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

  int min_value = process_total_time(procs_info[0].pid) - procs_info[0].executed_time;
  int current_index = -1;

  for (int i = 0; i < procs_count; i++) {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time <= min_value){
      min_value = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      current_index = i;
    }
  }

  if (current_index != -1)
    return procs_info[current_index].pid;

  return curr_pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  static int slice = 1;
  static int current_index = 0;

  if (!((++slice) % 5)) {
    current_index++;
  }

  current_index %= procs_count;
  
  return procs_info[current_index].pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  
  const int slice_time = 4;
  const int priority_boost = 8;
  
  static int current_time = 0;
  static int declared_queue = 0;
  static int last_max_pid = -1;

  int time, count;

  if (!declared_queue){
    for (int i = 0; i < NUM_QUEUES; i++) {
      init_queue(i, 200);
    }

    declared_queue = 1;
  }

  last_max_pid = update_queues(procs_info, procs_count, last_max_pid);

  if(current_time % priority_boost == 0) {
    // Subir de nivel al llegar al priority_boost

    for(int qid = 1; qid < NUM_QUEUES; qid++) {
        count = queues[qid].count;

        for(int i = 0; i < count; i++) {
            proc_info_t proc = dequeue(qid);
            enqueue(0, proc, 0);
        }
    }
  }
  else {
    // Bajar de nivel al llegar al slice_time

    for(int qid = 0; qid < NUM_QUEUES - 1; qid++) {
        count = queues[qid].count;

        for(int i = 0; i < count; i++) {
            time = queues[qid].executed_time[0];

            if(time >= slice_time) {
              proc_info_t proc = dequeue(qid);
              enqueue(qid + 1, proc, 0);
            }
        }
    }
  }

  // Ejecutar RR
  for(int i = 0; i < NUM_QUEUES; i++) {
    proc_info_t *temp = (proc_info_t*)malloc(queues[i].count * sizeof(proc_info_t));
    int pos = 0;
    
    if(queues[i].count) {

      for(int j = 0; j < queues[i].count; j++) {
        if(!queues[i].procs[j].on_io) {
          temp[pos++] = queues[i].procs[j];
        }
      }


      if(!pos) {
        continue;
      }

      curr_pid = rr_scheduler(temp, pos, curr_time, curr_pid);
      current_time++;

      for(int j = 0; j < queues[i].count; j++) {
        if(queues[i].procs[j].pid == curr_pid) {
          queues[i].executed_time[j]++;
          break;
        }
      }

      free(temp);
      break;
    }
    free(temp);
  }

  return curr_pid;
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

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
