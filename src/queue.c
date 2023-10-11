#include "queue.h"
#include "simulation.h"

#define NUM_QUEUES 3

void init_queue(int qid, int capacity) {
  queues[qid]->procs = (proc_info_t *)malloc(capacity * sizeof(proc_info_t));
  queues[qid]->count = 0;
  queues[qid]->capacity = capacity;
}

void enqueue(int qid, proc_info_t proc, int time) {
  if (queues[qid]->count == queues[qid]->capacity) {
    queues[qid]->capacity *= 2;
    queues[qid]->procs = (proc_info_t *)realloc(queues[qid]->procs, queues[qid]->capacity * sizeof(proc_info_t));
    queues[qid]->executed_time = (int *)realloc(queues[qid]->executed_time, queues[qid]->capacity * sizeof(int));
  }

  queues[qid]->executed_time[queues[qid]->count] = time;
  queues[qid]->procs[queues[qid]->count++] = proc;
}

proc_info_t dequeue(int qid) {
  proc_info_t proc = queues[qid]->procs[0];

  for (int i = 0; i < queues[qid]->count - 1; i++) {
    queues[qid]->procs[i] = queues[qid]->procs[i + 1];
    queues[qid]->executed_time[i] = queues[qid]->executed_time[i + 1];

  }
  queues[qid]->count--;

  return proc;
}

int update_queues(proc_info_t *procs_info, int procs_count, int last_max_pid) {
    int time, count;

    for(int qid = 0; qid < NUM_QUEUES; qid++) {
        count = queues[qid]->count;

        for(int i = 0; i < count; i++) {
            time = queues[qid]->executed_time[0];
            proc_info_t proc = dequeue(qid);

            for(int j = 0; j < procs_count; j++) {
                if(procs_info[i].pid == proc.pid) {
                    enqueue(qid, proc, time + 1);
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

    return max_pid;
}