#ifndef AUXILIAR
#define AUXILIAR

#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int find_pid_array(proc_info_t *procs_info, int procs_count, int pid);

int select_item(proc_info_t *procs, int count, int comparer(proc_info_t, proc_info_t));

void q_sort(void *arr, int count, size_t size, int (*compare)(const void *, const void *));

// MLFQ
void mlfq_merge(int past_pid[], int level_pid[], int time_pid[], proc_info_t *current_procs, int *past_count, int procs_count);

void mlfq_priority_bost(int levels[], int time_level[], int count);

int mlfq_update_proc(int pid[], int level[], int time_level[], int MAX_LEVEL, int MAX_TIME_LEVEL, int TIME, int index);

int mlfq_find_lowest_depth(proc_info_t *procs, int level[], int count, int *temp, int MAX_DEPTH, int current_pid);

void mlfq_filter_procs_level(proc_info_t *current_procs, int level[], int count, int depth, proc_info_t destiny[], int diferent_pid);

#endif