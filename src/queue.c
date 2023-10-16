#include "queue.h"

#include <stdlib.h>
#include <stdbool.h>

void start_queue(queue_t* q){
    q->first = q->last = q->nxt = NULL;
    q->len = 0;
}

void append_queue(queue_t* q, proc_info_t p){
    if(q->len == 0) {
        q->first->value = p;
        q->first->next = q->last = (node_t*) malloc(sizeof(node_t));
        q->nxt = q->first;
    }

    else{
        q->last->value = p;
        q->last->next = (node_t*) malloc(sizeof(node_t));
        q->last = q->last->next;
    }

    q->len++;
}

void remove_queue(queue_t* q, int curr_pid){
    if(q->len > 0){
        bool ok = false;

        if(curr_pid == q->first->value.pid) {
            q->first = q->first->next;
            ok = true;
        }
        
        else 
          ok = remove_node(q->first, q->last, curr_pid);

        if(ok) q->len--;
    }
}

bool remove_node(node_t* n, node_t* end, int curr_pid){
    if(curr_pid == n->next->value.pid) {
        n->next = n->next->next;
        return true;
    }

    if(n->next != end) return remove_node(n->next, end, curr_pid);

    return false;
}

bool find_queue(queue_t* q, proc_info_t p){
    return find_node(q->first, q->last, p);
}

bool find_node(node_t* n, node_t* end, proc_info_t p){
    if(p.pid == n->value.pid) return true;

    if(n->next != end) return find_node(n->next, end, p);

    return false;
}
