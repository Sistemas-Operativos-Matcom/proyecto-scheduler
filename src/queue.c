#include "queue.h"

#include <stdlib.h>
#include <stdio.h>

void start_queue(queue_t* q) {
    q->head = q->tail = NULL;
    q->len = 0;
}

void push_item(queue_t* q, proc_info_t value) {
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

proc_info_t pop_item(queue_t* q) {
    proc_info_t ans = q->head->value;
    q->head = q->head->next;
    q->len --;

    if (q->len == 0)q->head = q->tail = NULL;

    return ans;
}

