#include "process.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

process_t empty_process() {
  process_t process = {
      (int *)malloc(MAX_EV_PER_PROCESS * sizeof(int)),
      0,
      0,
      0,
  };
  return process;
}

process_t *load_from_file(char *file_path, int *count) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  size_t read;

  fp = fopen(file_path, "r");
  assert(fp);

  process_t *processes =
      (process_t *)malloc(MAX_PROCESS_COUNT * sizeof(process_t));
  process_t proc = empty_process();
  int proc_idx = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (line[read - 1] == '\n') {
      line[read - 1] = 0;
    }

    char *item;
    item = strtok(line, " ");
    proc.arrive_time = atoi(item);

    while ((item = strtok(NULL, " ")) != NULL) {
      proc.event_durations_msecs[proc.ev_count] = atoi(item);
      proc.total_duration_msecs += proc.event_durations_msecs[proc.ev_count];
      proc.ev_count++;
    }

    processes[proc_idx++] = proc;
    proc = empty_process();
  }

  fclose(fp);
  if (line) {
    free(line);
  }

  if (proc.ev_count > 0) {
    processes[proc_idx++] = proc;
  }
  *count = proc_idx;
  return processes;
}
