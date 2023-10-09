#ifndef AUXILIAR
#define AUXILIAR

#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int find_pid_array(proc_info_t *procs_info, int procs_count, int pid);

int select_item(void *arr, int count, size_t size, int (*selector)(const void *, const void *));

void q_sort(void *arr, int count, size_t size, int (*compare)(const void *, const void *));

#endif