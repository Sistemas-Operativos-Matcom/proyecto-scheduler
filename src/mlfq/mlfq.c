#include <stdlib.h>
#include "mlfq.h"

#define LEVELS_COUNT 3
#define MAX_JOBS 1000

mlfq new_mlfq()
{
    mlfq _mlfq = (mlfq)malloc(sizeof(struct mlfq));
    _mlfq->last_executed_pid = -1;
    _mlfq->time_slices = (int *)malloc(LEVELS_COUNT * sizeof(int));
    _mlfq->levels = (job_queue *)malloc(LEVELS_COUNT * sizeof(job_queue));

    // Initializing time slices and queues
    for (int i = 0; i < LEVELS_COUNT; i++)
    {
        if (i == 0)
            _mlfq->time_slices[i] = 20;
        else
            _mlfq->time_slices[i] = _mlfq->time_slices[i - 1] + 40;

        _mlfq->levels[i] = new_job_queue(MAX_JOBS);
    }

    return _mlfq;
}

int *search(mlfq _mlfq, int pid)
{
    int *coordinates = (int *)malloc(2 * sizeof(int));

    for (int i = 0; i < LEVELS_COUNT; i++)
    {
        job_queue level = _mlfq->levels[i];
        for (int j = 0; j < level->count; j++)
        {
            job current_job = level->jobs[j];
            if (current_job->pid == pid)
            {
                coordinates[0] = i;
                coordinates[1] = j;
                return coordinates;
            }
        }
    }

    coordinates[0] = coordinates[1] = -1;
    return coordinates;
}

void mlfq_remove(mlfq _mlfq, int pid)
{
    int *coordinates = search(_mlfq, pid);

    if (coordinates[0] == -1 || coordinates[1] == -1)
        fprintf(stderr, "MLFQ does not contain that job \n"), exit(1);

    job_queue_remove(_mlfq->levels[coordinates[0]], coordinates[1]);

    free(coordinates);
}

bool mlfq_contains(mlfq _mlfq, int pid)
{
    int *coordinates = search(_mlfq, pid);
    int i = coordinates[0], j = coordinates[1];
    free(coordinates);
    return i != -1 && j != -1;
}

void mlfq_add(mlfq _mlfq, int pid)
{
    // Adding new job to highest priority queue
    enqueue_job(_mlfq->levels[0], new_job(pid));
}

int mlfq_get(mlfq _mlfq, int io_pid, int interval_length)
{
    job next_job = NULL;
    int level_number = -1;

    for (int i = 0; i < LEVELS_COUNT; i++)
    {
        job_queue level = _mlfq->levels[i];

        if (level->count > 0)
        {
            for (int j = 0; j < level->count; j++)
            {
                // Checking if the next job is on I/O
                job temp = level->jobs[j];
                if (temp->pid != io_pid)
                {
                    next_job = temp;
                    break;
                }
            }

            if (next_job == NULL)
                continue;

            // Updating next job's ellapsed time
            next_job->ellapsed_slice_time += interval_length;

            // Checking if the next job consumed all the time-slice associated with its current priority level and if it's not on the lowest priority level
            if (next_job->ellapsed_slice_time >= _mlfq->time_slices[i] && i < LEVELS_COUNT - 1)
            {
                mlfq_remove(_mlfq, next_job->pid);
                level_number = i;
            }
            else
            {
                // Storing next job at the end of its current queue (level), for Round Robin purposes
                mlfq_remove(_mlfq, next_job->pid);
                enqueue_job(level, next_job);
            }
            break;
        }
    }

    // If there's no runnable job (note that jobs on IO are not runnable), then execute none
    if(next_job == NULL)
        return -1;

    // Lowering next job priority level
    if (level_number != -1)
    {
        next_job->ellapsed_slice_time = 0;
        enqueue_job(_mlfq->levels[level_number + 1], next_job);
    }

    _mlfq->last_executed_pid = (next_job == NULL) ? -1 : next_job->pid;

    return _mlfq->last_executed_pid;
}

void mlfq_boost(mlfq _mlfq)
{
    // Reseting ellapsed time-slices on first priority level
    job_queue first_level = _mlfq->levels[0];
    for (int i = 0; i < first_level->count; i++)
        first_level->jobs[i]->ellapsed_slice_time = 0;

    // Boosting lower priority levels
    for (int i = 1; i < LEVELS_COUNT; i++)
    {
        job_queue level = _mlfq->levels[i];
        while (level->count > 0)
        {
            job _job = dequeue_job(level);
            _job->ellapsed_slice_time = 0;
            enqueue_job(first_level, _job);
        }
    }
}

int* mlfq_processes(mlfq _mlfq, int* count)
{
    int* job_pids = (int*)malloc(MAX_JOBS * LEVELS_COUNT * sizeof(int));
    int k = 0;
    for(int i = 0; i < LEVELS_COUNT; i++)
    {
        job_queue level = _mlfq->levels[i];
        for(int j = 0; j < level->count; j++)
            job_pids[k++] = level->jobs[j]->pid, (*count)++;
    }
    return job_pids;
}