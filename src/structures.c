# include "structures.h"
# include <stdlib.h>
# include "simulation.h"

queue_t* build_queue()
{
    queue_t* q = malloc(sizeof(queue_t));
    q->count = 0;
    q->arr = (int*)malloc(sizeof(int)*100000);
    return q;
}

void push(queue_t* q, int p)
{
    q->arr[q->count] = p;
    q->count = (q->count+1);
}

int pop(queue_t* q)
{
    int proc = q->arr[0];
    for(int i = 0; i < q->count - 1; i++)
    {
        q->arr[i] = q->arr[i+1];
    }
    q->count--;
    return proc;
}