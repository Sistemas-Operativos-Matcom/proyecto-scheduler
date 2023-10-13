#if !defined(MLFQ)
#include <stdlib.h>
#include "bool.h"
#include "job_queue.h"

struct mlfq
{
    job_queue *levels;
    int last_executed_pid;
    int *time_slices;
};
typedef struct mlfq *mlfq;

// Constructor
mlfq new_mlfq();
// Removes the job with the input pid. Throws an exception if that job doesn't exist
void mlfq_remove(mlfq, int);
// Verifies if a job is present in the mlfq
bool mlfq_contains(mlfq, int);
// Adds a job to the mlfq, on its highest priority level
void mlfq_add(mlfq, int);
// Returns the pid of the next process to execute according to mlfq scheduling politics
int mlfq_get(mlfq, int, int);
// Boost all processes to satisfy mlfq scheduling politics
void mlfq_boost(mlfq);
// Returns an array containing all the job's pids contained in the mlfq
int* mlfq_processes(mlfq, int*);

#endif