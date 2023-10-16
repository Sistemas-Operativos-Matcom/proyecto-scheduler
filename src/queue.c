#include "queue.h"
queue_t *createQueue(unsigned capacity) {
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->length = 0;
    queue->array = (proc_info_t *)malloc(queue->capacity * sizeof(proc_info_t));
    return queue;
}

void enqueue(queue_t *queue, proc_info_t process) {
    if (queue->rear == queue->capacity - 1) {
        printf("La cola está llena.\n");
        return;
    }
    queue->length++;
    queue->array[++queue->rear].executed_time = process.executed_time;
    queue->array[queue->rear].on_io = process.on_io;
    queue->array[queue->rear].pid = process.pid;
    queue->array[queue->rear].time_slice_mlfq = process.time_slice_mlfq;
}

proc_info_t dequeue(queue_t *queue) {
    if (queue->front > queue->rear) {
        proc_info_t empty;
        empty.pid = -1;
        return empty;
    }
    queue->length--;
    return queue->array[queue->front++];
}