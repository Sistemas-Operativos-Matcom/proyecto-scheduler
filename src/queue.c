// queue.c
#include "queue.h"   // Incluimos la definición de la cola
#include <stdlib.h>

// Función para crear una nueva cola
Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));  // Asignamos memoria para la cola
    queue->front = queue->rear = NULL;   // Inicializamos los punteros de la cola
    return queue;
}

// Función para encolar un elemento en la cola
void enqueue(Queue* queue, int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));   // Creamos un nuevo nodo
    newNode->data = data;   // Asignamos los datos al nuevo nodo
    newNode->next = NULL;   // El nuevo nodo se coloca al final, así que su siguiente es NULL

    // Si la cola está vacía, el nuevo nodo será el único en la cola
    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
        return;
    }

    // Enlazamos el nodo al final de la cola y actualizamos el puntero rear
    queue->rear->next = newNode;
    queue->rear = newNode;
}

// Función para desencolar un elemento de la cola
int dequeue(Queue* queue) {
    // Si la cola está vacía, devolvemos -1 para indicar un error
    if (queue->front == NULL)
        return -1;

    Node* temp = queue->front;   // Guardamos el nodo frontal temporalmente
    int data = temp->data;   // Obtenemos los datos del nodo frontal

    queue->front = queue->front->next;   // Movemos el puntero frontal al siguiente nodo

    // Si después de desencolar la cola está vacía, actualizamos el puntero rear también
    if (queue->front == NULL)
        queue->rear = NULL;

    free(temp);   // Liberamos la memoria del nodo desencolado
    return data;
}
