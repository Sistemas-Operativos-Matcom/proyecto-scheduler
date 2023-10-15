#include <stdio.h>
#include <stdlib.h>

struct queue {
  int front, rear, size, capacity; // Variables para la cola, el frente, la parte trasera, el tamaño actual y la capacidad.
  int *arrayTime; // Arreglo para almacenar los tiempos de los procesos.
  int *process; // Arreglo para almacenar los PID de los procesos.
};

// Función para crear una nueva cola con una capacidad 'length'.
struct queue* createQueue(int length) {
  struct queue* queue = (struct queue*)malloc(sizeof(struct queue));
  queue->capacity = length;
  queue->front = queue->size = 0;
  queue->rear = -1;
  queue->arrayTime = (int*)malloc(length * sizeof(int));
  queue->process = (int*)malloc(length * sizeof(int));
  return queue;
}

// Función para verificar si la cola está vacía.
int e_queue(struct queue *queue) {
  return !size(queue); // Devuelve verdadero si el tamaño es 0.
}

// Función para obtener el tamaño actual de la cola.
int size(struct queue* queue) {
  return queue->size;
}

// Función para obtener el PID (proceso) en el frente de la cola.
int f_pid(struct queue* queue) {
  return size(queue) == 0 ? -1 : queue->process[queue->front];
}

// Función para obtener el tiempo en el frente de la cola.
int f_time(struct queue* queue) {
  return size(queue) == 0 ? -1 : queue->arrayTime[queue->front];
}

// Función para actualizar el tiempo en el frente de la cola.
int u_time(struct queue* queue) {
  if (size(queue) == 0) {
    return -1;
  }
  queue->arrayTime[queue->front] += 10;
  return queue->arrayTime[queue->front];
}

// Función para agregar un elemento a la cola con tiempo 't' y PID 'p'.
void push(struct queue* queue, int t, int p) {
  if (size(queue) == queue->capacity) {
    return; // Si la cola está llena, no hace nada.
  }
  queue->rear = (queue->rear + 1) % queue->capacity;
  queue->process[queue->rear] = p;
  queue->arrayTime[queue->rear] = t;
  queue->size++;
}

// Función para eliminar un elemento del frente de la cola.
void pop(struct queue* queue) {
  if (size(queue) == 0) {
    return; // Si la cola está vacía, no hace nada.
  }
  queue->process[queue->front] = -1;
  queue->arrayTime[queue->front] = -1;
  queue->front = (queue->front + 1) % queue->capacity;
  queue->size--;
}

// Función para verificar si un proceso con un PID específico está en la cola.
int PidQueue(struct queue* queue, int pid) {
  for (int i = 0; i < size(queue); i++) {
    if (pid == queue->process[(queue->front + i) % queue->capacity]) {
      return 1; // Devuelve 1 si se encuentra el PID en la cola.
    }
  }
  return 0; // Devuelve 0 si no se encuentra el PID en la cola.
}

// Función para transferir elementos de 'queue2' a 'queue1'.
void queue2toqueue1(struct queue* queue2, struct queue* queue1) {
  while (size(queue2) > 0) {
    int pid = f_pid(queue2); // Obtiene el PID de 'queue2'.
    pop(queue2); // Elimina el elemento del frente de 'queue2'.
    push(queue1, 0, pid); // Agrega el PID a 'queue1' con tiempo 0.
  }
}
