#ifndef QUEUE_H
#define QUEUE_H

typedef struct node
{
    int value;
    struct node* next;
    struct node* prev;
}node_t;

typedef struct queue
{
    node_t *head, *tail;
    int len;
}queue_t;

void start_queue(queue_t* q); //se puede usar para limpiar la cola
void push_item(queue_t* q, int value);
int pop_item(queue_t* q);

#endif