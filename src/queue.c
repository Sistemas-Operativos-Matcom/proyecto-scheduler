#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

void enqueue ( proc_info_t *process, struct Queue *queue )
{
    queue->rear++;
    queue->processes[queue->rear] = process;
}

void dequeue ( proc_info_t *process, struct Queue *queue )
{
    int aux=0;
    for(int i = 0 ; i <= queue->rear ; i++)
    {
        if(queue->processes[i]->pid == process[i].pid)
        {
            aux = 1 ;
            continue;
        }
        if(aux==1)
        {
            queue->processes[i-1]=queue->processes[i];

        }
    }

    queue->rear--;
}