#include "queue.h"

int isEmpty(queue_t* q) {
    return q->front == NULL ? 1 : 0;
}

queue_t createQueue() {
    queue_t q = malloc(sizeof(queue_t));
    q->front = NULL;
    q->last = NULL;

    return q;
}

void insertQueue(queue_t* q, int pid) {
    node_t newNode = malloc(sizeof(node_t));
    newNode->pid = pid;
    newNode->next = NULL;

    if(isEmpty(q)) {
        q->front = newNode;
        q->last = newNode;
    }
    else {
        q->last->next = newNode;
        q->last = newNode;
    }
}

int popQueue(queue_t q){
    if(isEmpty(q)){
        return NULL;
    }

    node_t* node = q->front;
    
    int pid = node->pid;
    q->front = node->next;

    free(node);

    return pid;
}