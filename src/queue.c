// #include "queue.h"

// // Función para inicializar una cola
// queue_t *createQueue(unsigned capacity) {
//     queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
//     queue->capacity = capacity;
//     queue->front = 0;
//     queue->rear = -1;
//     queue->array = (proc_info_t *)malloc(queue->capacity * sizeof(proc_info_t));
//     return queue;
// }

// // Función para encolar un proceso en la cola
// void enqueue(queue_t *queue, proc_info_t process) {
//     if (queue->rear == queue->capacity - 1) {
//         printf("La cola está llena.\n");
//         return;
//     }
//     queue->array[++queue->rear] = process;
// }

// // Función para desencolar un proceso de la cola
// proc_info_t dequeue(queue_t *queue) {
//     if (queue->front > queue->rear) {
//         proc_info_t empty;
//         empty.pid = -1;
//         return empty;
//     }
//     return queue->array[queue->front++];
// }
