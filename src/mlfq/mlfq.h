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

mlfq new_mlfq();
void mlfq_remove(mlfq, int);
bool mlfq_contains(mlfq, int);
void mlfq_add(mlfq, int);
int mlfq_get(mlfq, int, int);
void mlfq_boost(mlfq);

#endif