#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

void initializeQueue(struct Queue* q,int capacity)
{
    q->front = -1;
    q->rear = -1;
    q->size = 0;
    q->processes_pid = (int*) malloc(capacity * sizeof( int ));
}

int isEmpty(struct Queue* q)
{
    return q->size == 0;
}

void enqueue ( int pid, struct Queue* q )
{
    if (isEmpty(q)) 
    {
        q->front = 0;
    }

    q->rear++;
    q->size++;
    q->processes_pid[q->rear] = pid;
}

int dequeue ( struct Queue* q )
{
    if(isEmpty(q)) return -1;
    int p = q->processes_pid[q->front];
    if (q->front == q->rear) 
    {
        q->front = -1;
        q->rear = -1;
        q->size = 0;
    } else 
    {
        q->front++;
        q->size--;
    }
    return p;
}


int find_process( struct Queue* q,int pid)
{
    for(int i = q->front ; i <= q->rear;i++)
    {
        if(q->processes_pid[i] == pid)
        {
            return 1;
        }
    }
    return 0;
}