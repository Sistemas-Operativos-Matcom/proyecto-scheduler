#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "simulation.h"
#include "queue.h"
#define MAX 40000000

void initQueue(struct Queue *q)
{
    q->pop = -1;
    q->puntero = -1;
}
int isFull(struct Queue *q)
{
    return q->puntero == MAX - 1;
}
int isEmpty(struct Queue *q)
{
    return q->pop == -1;
}
void enqueue(struct Queue *q, proc_info_t value)
{
    if (isFull(q))
    {
        printf("La cola está llena\n");
    }
    else
    {
        if (q->pop == -1)
        {
            q->pop = 0;
        }
        q->puntero++;
        q->items[q->puntero] = value;
    }
}
proc_info_t dequeue(struct Queue *q)
{

    // asumimos que la cola no esta vacia
    proc_info_t item;

    item = q->items[q->pop];
    q->pop++;
    if (q->pop > q->puntero)
    {
        q->pop = q->puntero = -1;
    }

    return item;
}
int contains(struct Queue *q, proc_info_t item)
{
    for (int j = q->puntero; j < q->pop; j++)
    {
        if (q->items[j].pid == item.pid)
        {
            return 1;
        }
    }
    return 0;
}
int length(struct Queue *q)
{
    int count = 0;
    for (int i = q->puntero; i < q->pop; i++)
    {
        count++;
    }
    return count;
}
