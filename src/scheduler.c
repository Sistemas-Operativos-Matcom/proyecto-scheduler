#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

#define MAX_SIZE 10000
#define RR_SLICE 4

int *my_array;
int *my_array_count;

int *rr_slice;

//////////////////////////////////
/// MLFQ RELATED BEGIN
//////////////////////////////////
#define NUMBER_QUEUES 4
#define PRIORITY_BOOST 200

struct process_mlfq
{
    int pid;
    int rr_slice;
};

struct process_mlfq *array_queues[NUMBER_QUEUES];
int *arrays_len;
int *priority_queues;
int *time_passed;

void sync_ds_mlfq(proc_info_t *proc_info, int procs_count);

int my_mlfq(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

//////////////////////////////////
/// MLFQ RELATED END
//////////////////////////////////

void sync_ds(proc_info_t *proc_info, int procs_count);

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid);

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

    if (strcmp(name, "stcf") == 0)
    {
        my_array = (int *)malloc(sizeof(int) * MAX_SIZE);
        my_array_count = (int *)malloc(sizeof(int));
        *my_array_count = 0;
        return *my_preemptitive_shortest_job_first;
    }

    if (strcmp(name, "mlfq") == 0)
    {
        arrays_len = (int *)malloc(sizeof(int) * NUMBER_QUEUES);
        priority_queues = (int *)malloc(sizeof(int) * NUMBER_QUEUES);
        time_passed = (int *)malloc(sizeof(int));
        *time_passed = 0;
        for (int i = 0; i < NUMBER_QUEUES; i++)
        {
            array_queues[i] = malloc(sizeof(struct process_mlfq) * MAX_SIZE);
            arrays_len[i] = 0;
        }
        int start_priority = 8;
        for (int i = 0; i < NUMBER_QUEUES; i++)
        {
            priority_queues[i] = start_priority;
        }
        return *my_mlfq;
    }
    fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
    exit(1);
}

void sync_ds(proc_info_t *proc_info, int procs_count)
{
    int index = 0;
    for (int i = 0; i < *my_array_count; i++)
    {
        int j = 0;
        int pid = my_array[i];
        for (; j < procs_count; j++)
        {
            if (proc_info[j].pid == my_array[i])
            {
                break;
            }
        }
        if (j < procs_count)
        {
            my_array[index] = pid;
            index++;
        }
    }

    for (int i = 0; i < procs_count; i++)
    {
        int j = 0;
        for (; j < index; j++)
        {
            if (proc_info[i].pid == my_array[j])
            {
                break;
            }
        }
        if (j == index)
        {
            my_array[index] = proc_info[i].pid;
            index++;
        }
    }
    *my_array_count = index;
}

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    // Se devuelve el PID del primer proceso de todos los disponibles (los
    // procesos están ordenados por orden de llegada).
    return procs_info[0].pid;
}

int my_fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    sync_ds(procs_info, procs_count);
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
    sync_ds(procs_info, procs_count);
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

    /*-
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
                    // new process: so reset slice to 0 and move to the start of my array.
                    (*rr_slice) = 0;
                    for (int l = i; l < *my_array_count - 1; l++)
                    {
                        my_array[l] = my_array[l + 1];
                    }
                    for (int l = *my_array_count - 1; l >= 1; l--)
                    {
                        my_array[l] = my_array[l - 1];
                    }
                    my_array[0] = pid_to_locate;
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
    sync_ds(procs_info, procs_count);
    if (*my_array_count == 0)
    {
        return -1;
    }

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
            if (actual_pid == procs_info[j].pid)
            {
                on_io = procs_info[j].on_io;
                break;
            }
        }
        int asummption = process_total_time(actual_pid);
        if (!on_io && (min_time < 0 || min_time > asummption))
        {
            min_time = asummption;
            answer_pid = my_array[i];
        }
    }
    return answer_pid;
}

int my_preemptitive_shortest_job_first(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    sync_ds(procs_info, procs_count);
    if (*my_array_count == 0)
    {
        return -1;
    }

    /*
    from all the processes that are in my queue run the one who has less remaining time and it's not on i/o mode.
    */
    int min_time = __INT_MAX__;
    int answer_pid = -1;
    for (int i = 0; i < *my_array_count; i++)
    {
        int actual_pid = my_array[i];
        // int on_io = 0;
        int j = 0;
        for (; j < procs_count; j++)
        {
            if (actual_pid == procs_info[j].pid)
            {
                // on_io = procs_info[j].on_io;
                break;
            }
        }
        int asummption = process_total_time(actual_pid) - procs_info[j].executed_time;
        if ((asummption <= min_time))
        {
            min_time = asummption;
            answer_pid = my_array[i];
        }
    }
    return answer_pid;
}

//////////////////////////////////
/// BEGIN MLFQ RELATED
//////////////////////////////////

void handle_priority_boost()
{
    if (*time_passed == PRIORITY_BOOST)
    {
        int index = arrays_len[0];
        for (int l = 1; l < NUMBER_QUEUES; l++)
        {
            for (int m = 0; m < arrays_len[l]; m++)
            {
                array_queues[0][index].pid = array_queues[l][m].pid;
                array_queues[0][index].rr_slice = 0;
                index++;
            }
            arrays_len[l] = 0;
        }
        *time_passed = 0;
        arrays_len[0] = index;
    }
    else
    {
        *time_passed += 1;
    }
}

void move_queue_down(int actual_queue, int position)
{
    int pid = array_queues[actual_queue][position].pid;
    for (int i = position; i < arrays_len[actual_queue] - 1; i++)
    {
        array_queues[actual_queue][i].pid = array_queues[actual_queue][i + 1].pid;
        array_queues[actual_queue][i].rr_slice = array_queues[actual_queue][i + 1].rr_slice;
    }
    arrays_len[actual_queue]--;
    actual_queue += (actual_queue != NUMBER_QUEUES - 1);
    array_queues[actual_queue][arrays_len[actual_queue]].pid = pid;
    array_queues[actual_queue][arrays_len[actual_queue]].rr_slice = 0;
    arrays_len[actual_queue] += 1;
}

void sync_ds_mlfq(proc_info_t *proc_info, int procs_count)
{
    /*
    first for is to keep only common elements.
    */
    for (int l = NUMBER_QUEUES - 1; l >= 0; l--)
    {
        int index = 0;
        for (int i = 0; i < arrays_len[l]; i++)
        {
            int j = 0;
            int pid = array_queues[l][i].pid;
            for (; j < procs_count; j++)
            {
                if (proc_info[j].pid == pid)
                {
                    break;
                }
            }
            if (j < procs_count)
            {
                array_queues[l][index].pid = array_queues[l][i].pid;
                array_queues[l][index].rr_slice = array_queues[l][i].rr_slice;
                index++;
            }
        }
        if (!l)
        {
            /*
            this adds the new ones in procss_count to the first queue.
            */
            for (int i = 0; i < procs_count; i++)
            {
                int found = 0;
                for (int k = 0; k < NUMBER_QUEUES; k++)
                {
                    int j = 0;
                    for (; j < arrays_len[k]; j++)
                    {
                        if (proc_info[i].pid == array_queues[k][j].pid)
                        {
                            found = 1;
                            break;
                        }
                    }
                }
                if (!found)
                {
                    array_queues[l][index].pid = proc_info[i].pid;
                    array_queues[l][index].rr_slice = 0;
                    index++;
                }
            }
        }
        arrays_len[l] = index;
    }
}

int my_mlfq(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    static int last_used_queue = -1; // to do round robin I  need to remember last queue.
    sync_ds_mlfq(procs_info, procs_count);
    handle_priority_boost();
    /*
    at this point what should be done is find the topmost priority queue with elements not in i/o mode, and send to execute the first one, but handle round and robin carefully.

    first step with round and robin is check if a process consumed its time slice, but to this I need to remember last used queue.
    */

    if (last_used_queue >= 0 && arrays_len[last_used_queue] > 0)
    {
        for (int i = 0; i < arrays_len[last_used_queue]; i++)
        {
            if (priority_queues[last_used_queue] == array_queues[last_used_queue][i].rr_slice)
            {
                move_queue_down(last_used_queue, i);
                break;
            }
        }
    }
    /*-
    at this point try to execute first process that is not in i/o. *Also I assume that if the process not ended it is in the first position in my data structure.*
    */
    for (int t = 0; t < NUMBER_QUEUES; t++)
    {
        for (int i = 0; i < arrays_len[t]; i++)
        {
            int pid_to_locate = array_queues[t][i].pid;
            int old_rr_slice = array_queues[t][i].rr_slice;
            for (int j = 0; j < procs_count; j++)
            {
                if (procs_info[j].pid == pid_to_locate && !procs_info[j].on_io)
                {
                    if (i != 0)
                    {
                        for (int l = i; l < arrays_len[t] - 1; l++)
                        {
                            array_queues[t][l].pid = array_queues[t][l + 1].pid;
                            array_queues[t][l].rr_slice = array_queues[t][l + 1].rr_slice;
                        }
                        for (int l = arrays_len[t] - 1; l >= 1; l--)
                        {
                            array_queues[t][l].pid = array_queues[t][l - 1].pid;
                            array_queues[t][l].rr_slice = array_queues[t][l - 1].rr_slice;
                        }
                        array_queues[t][0].pid = pid_to_locate;
                        array_queues[t][0].rr_slice = old_rr_slice;
                    }
                    array_queues[t][0].rr_slice++;
                    last_used_queue = t;
                    return pid_to_locate;
                }
            }
        }
    }
    last_used_queue = -1;
    return -1;
}

//////////////////////////////////
/// END MLFQ RELATED
//////////////////////////////////
