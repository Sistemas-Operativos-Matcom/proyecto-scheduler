#ifndef HEAP_H
#define HEAP_H

typedef struct heap_node
{
    int time_ms;
    int pid;
} heap_node_t;

typedef struct heap_ {
    heap_node_t* nodes;
    int n;
} heap_t;

void start_heap(heap_t* heap, int MAX_SIZE);
void add_node(heap_t* heap, int time_ms, int pid);
heap_node_t* get_head(heap_t* heap);
void pop_head(heap_t* heap);

#endif