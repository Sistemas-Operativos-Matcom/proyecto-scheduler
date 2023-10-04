#include "queue.h"

#include <stdlib.h>
#include <stdio.h>

void start_queue(queue_t* q) {
    q->head = q->tail = NULL;
    q->len = 0;
}

void push_item(queue_t* q, int value) {
    if (q->tail != NULL) {
        q->tail->next = (node_t*) malloc(sizeof(node_t));
        q->tail->next->value = value;
        q->tail->next->prev = q->tail;
        q->tail = q->tail->next;
    } 
    else {
        q->tail = q->head = (node_t*) malloc(sizeof(node_t));
        q->head->value = value;
        q->head->prev = NULL;
    }

    q->len ++;
}

int pop_item(queue_t* q) {
    int ans = q->tail->value;
    q->tail = q->tail->prev;
    q->len --;

    return ans;
}

