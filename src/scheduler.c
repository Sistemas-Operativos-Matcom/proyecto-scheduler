#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

#define MAX_SIZE 100

proc_info_t *a;
int *a_count_pointer;
int *last_used_PID_pointer;

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid);

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

schedule_action_t get_scheduler(const char *name)
{
    a = (proc_info_t *)malloc(sizeof(proc_info_t) * MAX_SIZE);
    a_count_pointer = (int *)malloc(sizeof(int));
    *a_count_pointer = 0;
    last_used_PID_pointer = (int *)malloc(sizeof(int));
    *last_used_PID_pointer = -1;
    if (strcmp(name, "fifo") == 0)
    {
        return *fifo_scheduler;
    }
    if (strcmp(name, "my_own") == 0)
    {
        return *my_own_scheduler;
    }
    fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
    exit(1);
}

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    if (last_used_PID_pointer[0] >= 0) // only first time it will be -1.
    {
        // update_last_process(*procs_info, procs_count);
        /////////////////////////////////////////////////
        // BEGIN
        /////////////////////////////////////////////////
        int indexmydata_str = *a_count_pointer;
        int indexprocs_info = procs_count;
        for (int i = 0; i < procs_count; i++)
        {
            if (procs_info[i].pid == curr_pid)
            {
                indexprocs_info = i;
                break;
            }
        }
        for (int i = 0; i < *a_count_pointer; i++)
        {
            if (a[i].pid == curr_pid)
            {
                indexmydata_str = i;
                break;
            }
        }

        if (indexprocs_info == procs_count)
        {
            /*
            process is not in the active processes, so it ended.
            */
            for (int i = indexmydata_str; i < *a_count_pointer - 1; i++)
            {
                a[i].executed_time = a[i + 1].executed_time;
                a[i].on_io = a[i + 1].on_io;
                a[i].pid = a[i + 1].pid;
            }
            (*a_count_pointer)--;
        }
        else
        {
            a[indexmydata_str].executed_time = procs_info[indexprocs_info].executed_time;
            a[indexmydata_str].on_io = procs_info[indexprocs_info].on_io;
        }

        /////////////////////////////////////////////////
        // END
        /////////////////////////////////////////////////
    }

    // add_new_processes(*procs_info, procs_count, curr_pid);
    /////////////////////////////////////////////////
    // BEGIN
    /////////////////////////////////////////////////
    for (int i = 0; i < procs_count; i++)
    {
        if (procs_info[i].pid != curr_pid)
        {   
            for (int j = 0; j <= (*a_count_pointer); j++)
            {
                if (j == *a_count_pointer)
                {
                    a[j].executed_time = procs_info[i].executed_time;
                    a[j].on_io = procs_info[i].on_io;
                    a[j].pid = procs_info[i].pid;
                    (*a_count_pointer)++;
                    
                }
                else
                {
                    if (procs_info[i].pid == a[j].pid)
                    {
                        break;
                    }
                }
            }
        }
    }

    // choose_next();
    // at this point all the new info is updated so the scheduler may update
    // last_PID which will be the one that will executed next.
    /////////////////////////////////////////////////
    // BEGIN
    /////////////////////////////////////////////////

    *last_used_PID_pointer = a[0].pid; // fifo always will execute first in the array.
    
    /////////////////////////////////////////////////
    // END
    /////////////////////////////////////////////////
    return *last_used_PID_pointer;
}

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
    // Se devuelve el PID del primer proceso de todos los disponibles (los
    // procesos están ordenados por orden de llegada).
    return procs_info[0].pid;
}