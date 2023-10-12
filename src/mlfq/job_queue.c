#include <stdio.h>
#include <stdlib.h>
#include "bool.h"
#include "job_queue.h"

job_queue new_job_queue(int max_count)
{
    job_queue _job_queue = (job_queue)malloc(sizeof(struct job_queue));
    _job_queue->max_count = max_count;
    _job_queue->count = 0;
    _job_queue->jobs = (job *)malloc(max_count * sizeof(job));

    // Initializing array
    for (int i = 0; i < max_count; i++)
        _job_queue->jobs[i] = NULL;

    return _job_queue;
}

void enqueue_job(job_queue _queue, job _job)
{
    if (_queue->count >= _queue->max_count)
        fprintf(stderr, "Queue overflow \n"), exit(1);

    _queue->jobs[_queue->count] = _job;
    _queue->count++;
}

job dequeue_job(job_queue _queue)
{
    if (_queue->count == 0)
        fprintf(stderr, "Queue empty \n"), exit(1);

    job first = _queue->jobs[0];

    // Filling the gap left when removing the first job
    for (int i = 1; i < _queue->count; i++)
        _queue->jobs[i - 1] = _queue->jobs[i];

    _queue->count--;

    return first;
}

void job_queue_remove(job_queue _queue, int target_index)
{
    if (target_index >= _queue->count || target_index < 0)
        fprintf(stderr, "Index outside of the bounds of the queue \n"), exit(1);

    for (int i = target_index; i < (_queue->count - 1); i++)
        _queue->jobs[i] = _queue->jobs[i + 1];
    _queue->count--;
}

job job_queue_first(job_queue _queue)
{
    if (_queue->count == 0)
        fprintf(stderr, "Empty queue \n");
    return _queue->jobs[0];
}

job job_queue_last(job_queue _queue)
{
    if (_queue->count == 0)
        fprintf(stderr, "Empty queue \n");
    return _queue->jobs[_queue->count - 1];
}