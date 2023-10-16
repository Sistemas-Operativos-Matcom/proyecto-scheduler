#include "queue.h"
#include <stdio.h>

queue_t InitCola()
{
    queue_t queue;
    queue.count = 0;
    return queue;
}
int IsEmpty (queue_t queue)
{
    return queue.count==0;
}
int IsFull(queue_t queue)
{
    return queue.count == MAX -1;
}

void push(queue_t *queue,int element)
{
    queue->list[queue->count] = element;
    queue->count = queue->count +1;
}
void delete (queue_t *queue, int pos)
{
    for (size_t i = pos; i < queue->count-1; i++)
    {
        queue->list[i]=queue->list[i+1];
    }
    queue->count = queue->count -1;
    
}
int pop(queue_t *queue, int position)
{
    int element = queue->list[position];
    delete(queue,position);
    return element;

}