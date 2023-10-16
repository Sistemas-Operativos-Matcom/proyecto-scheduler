#ifndef QUEUE_H
#define QUEUE_H
#define MAX 100
typedef struct
{
    int list[MAX];
    int count;
}queue_t;

queue_t InitCola();
int IsFull(queue_t cola);
int IsEmpty(queue_t cola);
void push(queue_t *cola,int element);
int pop(queue_t *cola,int position);


#endif 
