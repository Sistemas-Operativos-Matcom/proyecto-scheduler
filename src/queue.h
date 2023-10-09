#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
    int pid;
    struct node_t* next;
} node_t;

typedef struct Queue {
    struct node_t* front;
    struct node_t* last;
} queue_t;

#endif