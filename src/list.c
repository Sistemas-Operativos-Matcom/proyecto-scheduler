#include "list.h"
#include <stdlib.h>
#include <string.h>

Node *CreateNode(proc_info_t proc)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->process = proc;
    node->priority = 2;
    node->start_time = 0;
    node->exec_time = 0;
    node->next = NULL;
    return node;
}

void Change_Priority(Node* proc)
{
    if(proc->priority!=0)
    {
        proc->priority--;
    }
}

void Boost_Priority(List* list)
{
    int position=0;
    Node* pointer=list->head;
    while(position<list->length)
    {
        pointer->priority=2;
        pointer=pointer->next;
        position++;
    }
}

void DestroyNode(Node *proc)
{
    free(proc);
}

void InsertNode(List *list, proc_info_t process)
{
    Node *proc = CreateNode(process);
    Node *pointer = list->head;
    if (list->head == NULL)
    {
        list->head = proc;
    }
    else
    {
        while (pointer->next)
        {
            pointer = pointer->next;
        }
        pointer->next = proc;
    }
    list->length++;
}

int ObtainPosition(List* list, proc_info_t element)
{
    int position=0;
    Node* pointer=list->head;
    while(element.pid!=pointer->process.pid && position<list->length)
    {
        pointer = pointer->next;
        position++;
    }
    if(position==list->length)
    {
        return -1;
    }
    return position;
    
}

void EliminateNode(List *list, int n)
{
    if (list->head)
    {
        if (n == 0)
        {
            Node *eliminated = list->head;
            list->head = list->head->next;
            DestroyNode(eliminated);
            list->length--;
        }
        else if (n < list->length)
        {
            Node *pointer = list->head;
            int position = 0;
            while (position < (n - 1))
            {
                pointer = pointer->next;
                position++;
            }
            Node *eliminated = pointer->next;
            pointer->next = eliminated->next;
            DestroyNode(eliminated);
            list->length--;
        }
    }
}

void ActNode(List* list, int n, proc_info_t element)
{
    Node* pointer = list->head;
    int position=0;
    while(position<n)
    {
        pointer=pointer->next;
        position++;
    }
    pointer->process=element;
}

int Contains(proc_info_t* elements, proc_info_t x, int procs_count)
{
    for(int i=0; i<procs_count; i++)
    {
        if(elements[i].pid==x.pid)
        {
            return 1;
        }
    }
    return 0;
}

Node *Obtain(List *list, int n)
{
    int position = 0;
    Node *pointer = list->head;
    while (position < n && pointer->next)
    {
        pointer = pointer->next;
        position++;
    }
    return pointer;
}

void ActList(List* list, proc_info_t* procs_info, int procs_count, int slice_time, int curr_time, int pb_time)
{
    //Agregando nodos nuevos a la lista y actualizando los existentes
    for(int i=0; i<procs_count; i++)
    {
        proc_info_t element = procs_info[i];
        int position = ObtainPosition(list, element);
        if(position==-1)
        {
            InsertNode(list, element);
        }
        else
        {
            int n = ObtainPosition(list, element);
            ActNode(list, n, element);
        }
    }
    //Eliminando elementos finalizados de la lista
    for(int i=0; i<list->length; i++)
    {
        if(!Contains(procs_info, list[i].head->process, procs_count))
        {
            EliminateNode(list, i);
        }
    }
    //Actualizando prioridades de la lista
    for(int i=0; i<list->length; i++)
    {
        Node* act = Obtain(list,i);
        act->exec_time=act->process.executed_time-act->start_time;
        if(act->exec_time>=slice_time)
        {
            Change_Priority(act);
            act->start_time=act->process.executed_time;
            act->exec_time=0;
        }
    }
    //Ejecutando el Priority Boost si es necesario
    if(curr_time%slice_time==0)
    {
        Boost_Priority(list);
    }
}
