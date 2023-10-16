#include "scheduler.h"
#include "simulation.h"

typedef struct ProcessNode
{
    proc_info_t* process;
    struct ProcessNode* next; 
} ProcessNode;

typedef struct Queue
{
    ProcessNode *first;
    ProcessNode *last;
    int time_slice;
} Queue;

Queue* initialize();
void enqueue(Queue*, proc_info_t*);
proc_info_t* dequeue(Queue*);
