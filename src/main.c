#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "process.h"
#include "scheduler.h"
#include "simulation.h"

int timeP[200];

int main(int argc, char **argv) {
  char *path = argv[1];
  char *scheduler_name = argv[2];
  int config = argc > 3 ? parse_config(argv[3]) : 0;

  int process_count;
  process_t *processes = load_from_file(path, &process_count);
  printf("--------------------------------------------\n");
  printf("Case file: '%s'\n", path);
  printf("Simulating %d processes\n", process_count);
  printf("Scheduler: %s\n", scheduler_name);
  printf("--------------------------------------------\n\n");

  start_new_simulation(processes, process_count, get_scheduler(scheduler_name ,&timeP),
                       config);
  return 0;
}
