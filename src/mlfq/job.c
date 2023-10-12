#include <stdlib.h>
#include "job.h"

job new_job(int pid)
{
    job _job = (job)malloc(sizeof(struct job));

    _job->pid = pid;
    _job->ellapsed_slice_time = 0;

    return _job;
}