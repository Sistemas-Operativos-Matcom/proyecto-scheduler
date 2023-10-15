#ifndef list_h
#define list_h
#include <stdio.h>
#include "simulation.h"

typedef struct Node
{
    proc_info_t process;
    int priority;
    int start_time;
    int exec_time;
    struct Node* next;

}Node;

typedef struct List
{
    Node* head;
    int length;
}List;

void ActList(List* list, proc_info_t* procs_info, int procs_count, int slice_time, int curr_time, int pb_time);

#endif