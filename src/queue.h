// queue.h

// Definición de la estructura de un nodo en la cola
typedef struct Node {
    int data;           // Datos almacenados en el nodo
    struct Node* next;   // Puntero al siguiente nodo en la cola
} Node;

// Definición de la estructura de una cola
typedef struct {
    Node* front;    // Puntero al frente de la cola (primer nodo)
    Node* rear;     // Puntero a la parte trasera de la cola (último nodo)
} Queue;

// Prototipos de funciones para operar sobre la cola
Queue* createQueue();       // Crear una nueva cola
void enqueue(Queue* queue, int data);   // Encolar un elemento
int dequeue(Queue* queue);   // Desencolar un elemento