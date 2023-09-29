#include "heap.h"

#include <stdlib.h>

void start_heap(heap_t* heap, int MAX_SIZE) {
    heap->n = 0;
    heap->nodes = (heap_node_t*) malloc(MAX_SIZE * sizeof(heap_t));
}

void add_node(heap_t* heap, int time_ms, int pid) {
    heap->nodes[heap->n++] = (heap_node_t) {time_ms, pid};

    int k = heap->n - 1, l;
    heap_node_t tmp;

    while (k) {
        l = (k - 1) >> 1;

        if (heap->nodes[l].time_ms > heap->nodes[k].time_ms) {
            tmp = heap->nodes[l];
            heap->nodes[l] = heap->nodes[k];
            heap->nodes[k] = tmp;
        }
        else break;

        k = l;
    }
}

heap_node_t* get_head(heap_t* heap) {
    return &heap->nodes[0];
}

void pop_head(heap_t* heap) {
    heap->nodes[0] = heap->nodes[--heap->n];

    heap_node_t tmp;

    int k = 0, l, r, s;
    while (k < heap->n) {
        l = (k << 1) + 1;
        r = (k << 1) + 2;
        s = k;

        if (l < heap->n && heap->nodes[l].time_ms < heap->nodes[s].time_ms) s = l;
        if (r < heap->n && heap->nodes[r].time_ms < heap->nodes[s].time_ms) s = r;

        if (s == k)break;

        tmp = heap->nodes[k];
        heap->nodes[k] = heap->nodes[s];
        heap->nodes[s] = tmp;

        k = s;
    }
}
