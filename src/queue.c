#include "queue.h"
#include "simulation.h"
#include <stdlib.h>


Queue* initialize(int time_slice)
{
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->first = q->last = NULL;
    q->time_slice = time_slice;
    return q;
}

void enqueue(Queue* q, proc_info_t* process)
{
    ProcessNode *procNode = (ProcessNode*)malloc(sizeof(ProcessNode));
    
    if (procNode == NULL) {
        printf("Allocation failed\n");
        return;
    }
    procNode->process=process;
    procNode->next = NULL;

    if (q->first == NULL) {
        q->first = q->last = procNode;
    }
    else {
        q->last->next = procNode;
        q->last = procNode;
    }
}

proc_info_t* dequeue(Queue* q)
{
    if (q->first == NULL)
    {//cola vacia
        return;
    }
    ProcessNode* tempNode = q->first;
    proc_info_t* proc_pointer = tempNode->process;
    q->first = q->first->next;
    if (q->first == NULL)
    {
        q->last = NULL;
    }
    free(tempNode);
    return proc_pointer;
}