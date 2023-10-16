#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"
#include <limits.h>
#define HIGH_PRIORITY 0
#define MEDIUM_PRIORITY 1
#define LOW_PRIORITY 2
#define MAX_PROCESSES 10


int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    if(curr_pid == -1) 
        return procs_info[0].pid;
        
    int shortest_pid = -1;
    int shortest_duration = INT_MAX;

    for (int i = 0; i < procs_count; i++) {
        int pid = procs_info[i].pid;
        int duration = process_total_time(pid);
        if (!procs_info[i].on_io && duration < shortest_duration) {
            shortest_duration = duration;
            shortest_pid = pid;
        }
    }

    return shortest_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    if(curr_pid == -1) 
        return procs_info[0].pid;


    int st = INT_MAX;
    int pid = -1;

  for (int i = 0; i < procs_count; i++)
  {
    if (!procs_info[i].on_io && process_total_time(procs_info[i].pid) - procs_info[i].executed_time < st)
    {
      st = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      pid = procs_info[i].pid;
    }
  }

  return pid;
}


int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    int quantum = 5;

    if(curr_pid == -1) 
        return procs_info[0].pid;

    for (int i = 0; i < procs_count; i++) {

        if(procs_info[i].pid == curr_pid) {
          
            if(curr_time % quantum != 0) {
               return curr_pid;
            }

            return i != procs_count - 1 ? procs_info[i + 1].pid : procs_info[0].pid;  
        }
    }

    return -1;
}

// Estructura para mantener información sobre las colas MLFQ
typedef struct {
    int queue[3][MAX_PROCESSES];
    int front[3];
    int rear[3];
} MLFQ;

// Inicializar las colas MLFQ
void init_mlfq(MLFQ *mlfq) {
    for (int i = 0; i < 3; i++) {
        mlfq->front[i] = -1;
        mlfq->rear[i] = -1;
    }
}

// Agregar un proceso a la cola de MLFQ con la prioridad correspondiente
void enqueue(MLFQ *mlfq, int priority, int pid) {
    mlfq->rear[priority]++;
    mlfq->queue[priority][mlfq->rear[priority]] = pid;

    if (mlfq->front[priority] == -1) {
        mlfq->front[priority] = 0;
    }
}

int get_my_priority(int time) {
    if (time < 5) {
        return HIGH_PRIORITY;
    } else if (time < 10) {
        return MEDIUM_PRIORITY;
    } else {
        return LOW_PRIORITY;
    }
}


int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    MLFQ mlfq;
    init_mlfq(&mlfq);

    // Agregar procesos listos a las colas de MLFQ según su prioridad
    for (int i = 0; i < procs_count; i++) {
        int pid = procs_info[i].pid;
        if (!procs_info[i].on_io) {
            int priority = get_my_priority(procs_info[i].executed_time);
            enqueue(&mlfq, priority, pid);
        }
    }

    // Obtener el próximo proceso a ejecutar
    for (int i = HIGH_PRIORITY; i <= LOW_PRIORITY; i++) {
        if (mlfq.front[i] != -1) {
            return mlfq.queue[i][mlfq.front[i]++];
        }
    }

    return -1; // Si no se encuentra ningún proceso listo en ninguna cola
}

schedule_action_t get_scheduler(const char *name) {
    if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
    if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
    if (strcmp(name, "round_robin") == 0) return *round_robin_scheduler;
    if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;
    if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

    fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
    exit(1);
}
