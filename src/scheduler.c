#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

#define MAX_SIZE 1000
#define RR_SLICE 4

int *my_array;
int *my_array_count;

int *rr_slice;

void add_new_processes(proc_info_t *proc_info, int procs_count);
void remove_already_finished(proc_info_t *proc_info, int procs_count);

int my_fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

int my_round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

int my_no_preemptitive_shortest_job_first(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

int my_preemptitive_shortest_job_first(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

schedule_action_t get_scheduler(const char *name)
{
    if (strcmp(name, "fifo") == 0)
    {
        return *fifo_scheduler;
    }
    if (strcmp(name, "my_fifo") == 0)
    {
        my_array = (int *)malloc(sizeof(int) * MAX_SIZE);
        my_array_count = (int *)malloc(sizeof(int));
        *my_array_count = 0;
        return *my_fifo_scheduler;
    }

    if (strcmp(name, "rr") == 0)
    {
        my_array = (int *)malloc(sizeof(int) * MAX_SIZE);
        my_array_count = (int *)malloc(sizeof(int));
        *my_array_count = 0;
        rr_slice = (int *)malloc(sizeof(int));
        *rr_slice = 0;
        return *my_round_robin_scheduler;
    }

    if (strcmp(name, "sjf") == 0)
    {
        my_array = (int *)malloc(sizeof(int) * MAX_SIZE);
        my_array_count = (int *)malloc(sizeof(int));
        *my_array_count = 0;
        return *my_no_preemptitive_shortest_job_first;
    }

    if (strcmp(name, "psjf") == 0)
    {
        my_array = (int *)malloc(sizeof(int) * MAX_SIZE);
        my_array_count = (int *)malloc(sizeof(int));
        *my_array_count = 0;
        return *my_preemptitive_shortest_job_first;
    }

    fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
    exit(1);
}

void remove_already_finished(proc_info_t *proc_info, int procs_count)
{
    /*
    1 - remove from my data structure all the processes that already finished.
    */
    for (int i = 0; i < procs_count; i++)
    {
        int pid = proc_info[i].pid;
        int j = 0;
        for (; j < *my_array_count; j++)
        {
            if (my_array[j] == pid)
            {
                break;
            }
        }
        if (j < *my_array_count)
        {
            for (int l = j; l < *my_array_count - 1; l++)
            {
                my_array[l] = my_array[l + 1];
            }
            (*my_array_count)--;
        }
    }
}

void add_new_processes(proc_info_t *proc_info, int procs_count)
{
    for (int i = 0; i < procs_count; i++)
    {
        int pid = proc_info[i].pid;
        int j = 0;
        for (; j < *my_array_count; j++)
        {
            if (my_array[j] == pid)
            {
                break;
            }
        }

        if (j == *my_array_count)
        {
            my_array[j] = pid;
            (*my_array_count)++;
        }
    }
}

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    // Se devuelve el PID del primer proceso de todos los disponibles (los
    // procesos están ordenados por orden de llegada).
    return procs_info[0].pid;
}

int my_fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    remove_already_finished(procs_info, procs_count);
    add_new_processes(procs_info, procs_count);
    for (int i = 0; i < *my_array_count; i++)
    {
        int pid_to_locate = my_array[i];
        for (int j = 0; j < procs_count; j++)
        {
            if (procs_info[j].pid == pid_to_locate && !procs_info[j].on_io)
            {
                return pid_to_locate;
            }
        }
    }
    return -1;
}

int my_round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    remove_already_finished(procs_info, procs_count);
    add_new_processes(procs_info, procs_count);
    // there are no active process, shit.
    if (*my_array_count == 0)
    {
        *rr_slice = 0;
        return -1;
    }

    if (*rr_slice == RR_SLICE)
    {
        // only care about the case where the process ended its slice (was last executed process) and not ended (its still in my array), so we have to move it to the end.
        if (curr_pid == my_array[0])
        {
            // process ended its slice but not ended, so move to the end.
            int temp = my_array[0];
            for (int k = 0; k < *my_array_count - 1; k++)
            {
                my_array[k] = my_array[k + 1];
            }
            my_array[*my_array_count - 1] = temp;
        }
        *rr_slice = 0;
    }
    /*
    at this point try to execute first process that is not in i/o. *Also I assume that if the process not ended it is in the first position in my data structure.*
    */
    for (int i = 0; i < *my_array_count; i++)
    {
        int pid_to_locate = my_array[i];
        for (int j = 0; j < procs_count; j++)
        {
            if (procs_info[j].pid == pid_to_locate && !procs_info[j].on_io)
            {
                if (i != 0)
                {
                    // new process so reset slice to 0.
                    (*rr_slice) = 0;
                }
                (*rr_slice)++;
                return pid_to_locate;
            }
        }
    }
    /*
    all the processes are on i/o so don't do anything.
    */
    (*rr_slice) = 0;
    return -1;
}

int my_no_preemptitive_shortest_job_first(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    remove_already_finished(procs_info, procs_count);
    add_new_processes(procs_info, procs_count);

    if (*my_array_count == 0)
    {
        return -1;
    }

    /*
    if the last process in cpu is still active keep executing it, even though it can be on i/o mode.
    this makes it no-preemptitive.
    */
    for (int i = 0; i < *my_array_count; i++)
    {
        if (my_array[i] == curr_pid)
        {
            return curr_pid;
        }
    }

    /*
    from all the processes that are in my queue run the one that takes less time and it's not on i/o mode.
    */
    int min_time = -1;
    int answer_pid = -1;
    for (int i = 0; i < *my_array_count; i++)
    {
        int actual_pid = my_array[i];
        int on_io = 0;
        for (int j = 0; j < procs_count; j++)
        {
            if (actual_pid = procs_info[j].pid)
            {
                on_io = procs_info[j].on_io;
                break;
            }
        }
        int asummption = process_total_time(actual_pid);
        if (!on_io && (min_time < 0 || min_time > asummption))
        {
            min_time = asummption;
            answer_pid = i;
        }
    }
    return answer_pid;
}

int my_preemptitive_shortest_job_first(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    remove_already_finished(procs_info, procs_count);
    add_new_processes(procs_info, procs_count);

    if (*my_array_count == 0)
    {
        return -1;
    }

    /*
    from all the processes that are in my queue run the one that takes less time and it's not on i/o mode.
    */
    int min_time = -1;
    int answer_pid = -1;
    for (int i = 0; i < *my_array_count; i++)
    {
        int actual_pid = my_array[i];
        int on_io = 0;
        int j = 0;
        for (; j < procs_count; j++)
        {
            if (actual_pid = procs_info[j].pid)
            {
                on_io = procs_info[j].on_io;
                break;
            }
        }
        int asummption = process_total_time(actual_pid) - procs_info[j].executed_time;
        if (!on_io && (min_time < 0 || min_time > asummption))
        {
            min_time = asummption;
            answer_pid = i;
        }
    }
    return answer_pid;
}