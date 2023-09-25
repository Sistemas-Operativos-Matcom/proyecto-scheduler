#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESS_COUNT 10000
#define MAX_EV_PER_PROCESS 1000

typedef struct process {
  int *event_durations_msecs;
  int ev_count;
  int arrive_time;
  int total_duration_msecs;
} process_t;

process_t *load_from_file(char *path, int *count);
#endif
