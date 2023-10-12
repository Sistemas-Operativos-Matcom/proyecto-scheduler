#if !defined(JOB_QUEUE)
#include <stdio.h>
#include "job.h"
struct job_queue
{
    job *jobs;
    int count;
    int max_count;
};
typedef struct job_queue *job_queue;

job_queue new_job_queue(int);
void enqueue_job(job_queue, job);
job dequeue_job(job_queue);
void job_queue_remove(job_queue, int);
job job_queue_first(job_queue);
job job_queue_last(job_queue);

#endif