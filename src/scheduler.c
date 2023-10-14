#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h" 


// Data structure to represent a queue
struct queue
{
	int *items; 	// array to store queue elements
  int *proc_time; // array asociado al items con sus tiempo que llevo ejecutando el proceso
	int max_size;	// maximum capacity of the queue
	int front;  	// front points to the front element in the queue (if any)
	int last;   	// last points to the last element in the queue
	int size;   	// current capacity of the queue
};

// Utility function to initialize a queue
struct queue* new_queue(int size)
{
	struct queue *q = NULL;
	q = (struct queue*)malloc(sizeof(struct queue));

	q->items = (int*)malloc(size * sizeof(int));
	q->proc_time = (int*)malloc(size * sizeof(int));
	q->max_size = size;
	q->front = 0;
	q->last = -1;
	q->size = 0;

	return q;
}

// Utility function to return the size of the queue
int size(struct queue *q) {
	return q->size;
}

// Utility function to check if the queue is emqy or not
int is_empty(struct queue *q) {
	return !size(q);
}

// Utility function to return the front element of the queue
int front(struct queue *q)
{
	if (is_empty(q))
	{
		printf("Underflow by front\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	return q->items[q->front];
}

int add_slide_time(struct queue *q) {
  if (is_empty(q))
	{
		printf("Underflow by slide time\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	q->proc_time[q->front] += 10;
  return q->proc_time[q->front];
}

// Utility function to add an element `x` to the queue
void enqueue(struct queue *q, int x)
{
	if (size(q) == q->max_size)
	{
		printf("Overflow by enqueue\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	// printf("Inserting %d\t", x);

	q->last = (q->last + 1) % q->max_size;	// circular queue
	q->items[q->last] = x;
  q->proc_time[q->last] = 0;
	q->size++;

	// printf("front = %d, last = %d\n", q->front, q->last);
}

// Utility function to dequeue the front element
void dequeue(struct queue *q)
{
	if (is_empty(q))	// front == last
	{
		printf("Underflow by dequeue\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	// printf("Removing %d\t", front(q));

	q->front = (q->front + 1) % q->max_size;  // circular queue
	q->size--;

	// printf("front = %d, last = %d\n", q->front, q->last);
}

// revisa si un proceso esta en queue
int exist_in_queue(struct queue *q, int pid) {
  for (int i = 0; i < q->size; i++) {
    int pos = q->front + i;
    if (pid == q->items[pos]) {
      return 1;
    }
  } 
  return 0;
}

// revisa si el front de la queue esta en los procesos
int exist_in_procs(struct queue *q, proc_info_t *procs_info, int procs_count) {
  int pid = front(q);
  for (int i = 0; i < procs_count; i++) {
    int proc = procs_info[i].pid;
    if (pid == proc) {
      return 1;
    }
  }  
  return 0;
}


int get_proc_time(struct queue *q) {
  return q->proc_time[q->front];
}


// La función que define un scheduler está compuesta por los siguientes
// parámetros:
//
//  - procs_info: Array que contiene la información de cada proceso activo
//  - procs_count: Cantidad de procesos activos
//  - curr_time: Tiempo actual de la simulación
//  - curr_pid: PID del proceso que se está ejecutando en el CPU
//
// Esta función se ejecuta en cada timer-interruq donde existan procesos
// activos (se asegura que `procs_count > 0`) y determina el PID del proceso a
// ejecutar. El valor de retorno es un entero que indica el PID de dicho
// proceso. Pueden ocurrir tres casos:
//
//  - La función devuelve -1: No se ejecuta ningún proceso.
//  - La función devuelve un PID igual al curr_pid: Se mantiene en ejecución el
//  proceso actual.
//  - La función devuelve un PID diferente al curr_pid: Simula un cambio de
//  contexto y se ejecuta el proceso indicado.
//
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid) {
//   // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
//   // cambiar)

//   // Información que puedes obtener de un proceso
//   int pid = procs_info[0].pid;      // PID del proceso
//   int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
//                                     // realizando una opreación IO
//   int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
//                                                 // ejecutado (en CPU o en I/O)

//   // También puedes usar funciones definidas en `simulation.h` para extraer
//   // información extra:
//   int duration = process_total_time(pid);

//   return -1;
// }
 
 

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  // ejecuta el mismo mientras no termine
  if (curr_pid != -1) {
    return curr_pid;
  }
  int min = 1e9;
  int send = -1;
  int pos = 0;
  while (pos < procs_count) {
    int pid = procs_info[pos].pid;
    int duration = process_total_time(pid);
    if (duration < min) {
      min = duration;
      send = pid;
    }
    pos++;
  }
  return send;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {  
  // en cada momento seleciona el proceso que menos tiempo le quede
  int min = 1e9;
  int send = -1;
  int pos = 0;
  while (pos < procs_count) {
    int pid = procs_info[pos].pid;
    int duration = process_total_time(pid) - procs_info[pos].executed_time;
    if (duration < min) {
      min = duration;
      send = pid;
    }
    pos++;
  }
  return send;
}

int push_all_procs(proc_info_t *procs_info, int procs_count, struct queue *q) {
  for (int i = 0; i < procs_count; i++) {
    int pid = procs_info[i].pid;
    if ( exist_in_queue(q, pid) == 0) {
      enqueue(q, pid);
    }
  }
}

int is_on_io(proc_info_t *procs_info, int procs_count, int pid) {
  for (int i = 0; i < procs_count; i++) {
    int pid = procs_info[i].pid;
    if ( procs_info[i].pid == pid) {
      if (procs_info[i].on_io == 1) {
      // printf("%i\n", pid);
      return 1;
      } else {
        return 0;
      }
    }
  }
  return -1;
}

// mueve el elemento del principio al final
int changeQueue(struct queue *q) {
    int pid = front(q); 
    dequeue(q);
    enqueue(q, pid); 
    return 1;
}



static struct queue *qr;
int proc_time = 50; // sera tomado esto como slide_time y como time_slide
int last_pid = -1;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // si llegan nuevos procesos ponlos en la cola 1
  push_all_procs(procs_info, procs_count, qr);
  
  if (curr_pid == -1 && last_pid != -1)
  {
    if (!is_empty(qr) && front(qr) == last_pid && size(qr) > 0)
    {
  // printf("%i\n", size(qr));
      dequeue(qr);
    }
  }  
  
  // si se acabo su tiempo cambia de proceso mandolo para el final de la cola
  if (!is_empty(qr) && get_proc_time(qr) >= proc_time ) {
    changeQueue(qr);
  }

  // para el caso de que borre todo de la cola pero todavia quedan procesos por hacer
  if (size(qr) == 0) { 
    for (int i = procs_count - 1; i >= 0; i--) {
      int pid = procs_info[i].pid;
    //  printf("%i\n", exist_in_queue(qr, pid));
      if (exist_in_queue(qr, pid) == 0) {
        enqueue(qr, pid);
        continue;
      }
      break;
    }
  }

  // cuando un proceso lo ultimo que hace es io la cola piensa que todavia existe entonces hay que borrarlo
  while (!exist_in_procs(qr, procs_info, procs_count)) {
    dequeue(qr);
  }

  // implementacion de io
  int cant = 0;
  while (is_on_io(procs_info, procs_count, front(qr)) && cant < size(qr)) {
    // printf("%i ", front(qr));
    if (is_on_io(procs_info, procs_count, front(qr)) == -1) {
      dequeue(qr);
    } else {
      int t = get_proc_time(qr);
      changeQueue(qr);
      qr->proc_time[qr->last] = t;
    }
    // printf("%i \n", front(qr));
    cant++;
    // printf("%i\n", front(q1));
  }
 
  // manda a ejecutar el proceso en el front de la cola
  last_pid = front(qr);
  add_slide_time(qr);
  return front(qr);
}
 

static struct queue *q1;
static struct queue *q2;
int pb = 400; // priority boost
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // revisemos que el proceso actual existe
  // printf("%i ", size(q1));
  if (curr_pid == -1 && last_pid != -1)
  {
    if (!is_empty(q1) && front(q1) == last_pid && size(q1) > 0)
    {
      dequeue(q1);
    }
    else
    {
      dequeue(q2);
    }
  }  
  // si llegan nuevos procesos ponlos en la cola 1
  for (int i = procs_count - 1; i >= 0; i--) {
    int pid = procs_info[i].pid;
    if (procs_info[i].executed_time == 0 && exist_in_queue(q1, pid) == 0 && exist_in_queue(q2, pid) == 0) {
      // printf("%i ", pid);
      enqueue(q1, pid);
      continue;
    }
    // printf("%i\n", exist_in_queue(q1, procs_info[i].pid));
    break;
  }
  // baja el proceso de q1 si se acabo su proc_time
  if (!is_empty(q1) && get_proc_time(q1) >= proc_time ) {
    int pid = front(q1);
    dequeue(q1);
    enqueue(q2, pid); 
  }

  // Revisar slide time en cola 2 para hacer rr
  if (!is_empty(q2) && get_proc_time(q2) >= proc_time ) {
    int pid = front(q2);
    dequeue(q2);
    enqueue(q2, pid); 
  }

  // si toca el priority boost sube todo a la cola 1
  if (curr_time % pb == 0) {
    while (!is_empty(q2)) {
      int pid = front(q2);
      dequeue(q2);
      enqueue(q1, pid);
    }
  }

  int all_io = 0;
  // decide que proceso ejecutar primero coje de la cola 1 y si esta vacia de la cola 2
  if (!is_empty(q1)) {
    // cojo el front aumento su proc_time y lo mando para el final de la queue 
    //  cuando un proceso lo ultimo que hace es io la cola piensa que todavia existe entonces hay que borrarlo
    while(size(q1) > 0 && !exist_in_procs(q1, procs_info, procs_count)) {
      dequeue(q1);
    }
 
    for (int i = 0; i < procs_count; i++) {
      int pid = procs_info[i].pid;
      if (exist_in_queue(q1, pid) == 0) {
        // printf("%i ", pid);
        enqueue(q1, pid);
        continue;
      }
      // printf("%i\n", exist_in_queue(q1, procs_info[i].pid));
    }

    // implementacion de io 
    int cant = 0;
    while (is_on_io(procs_info, procs_count, front(q1)) && cant < size(q1)) {
      // printf("%i ", front(qr));
      if (is_on_io(procs_info, procs_count, front(q1)) == -1) {
        dequeue(q1);
      } else {
        int t = get_proc_time(q1);
        changeQueue(q1);
        q1->proc_time[q1->last] = t;
      }
      // printf("%i \n", front(qr));
      cant++;
      // printf("%i\n", front(q1));
    }

    if (cant == size(q1)) {
      all_io = 1;
    }

    // printf("%i", size(q1)); 
    int pid = front(q1);
    add_slide_time(q1);
    last_pid = pid;
    return pid;
  } else {
    //  cuando un proceso lo ultimo que hace es io la cola piensa que todavia existe entonces hay que borrarlo
    while(size(q2) > 0 && !exist_in_procs(q2, procs_info, procs_count)) {
      dequeue(q2);
    }

    // pasa que algunos procesos no estan en la cola pero si existen 
    for (int i = 0; i < procs_count; i++) {
      int pid = procs_info[i].pid;
      if (exist_in_queue(q2, pid) == 0) {
        // printf("%i ", pid);
        enqueue(q2, pid);
        continue;
      }
      // printf("%i\n", exist_in_queue(q1, procs_info[i].pid));
    }
    //  printf("%i ", q2->size);
    int pid = front(q2);
    add_slide_time(q2);
    // printf("%i ", q2->size);
    last_pid = pid;
    return pid;
  }
  
  
}

// Esta función devuelve la función que se ejecutará en cada timer-interruq
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //_scheduler

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler; 
  if (strcmp(name, "rr") == 0){ 
    qr = new_queue(10000);
    return *rr_scheduler;
  }
  if (strcmp(name, "mlfq") == 0) {
    // colas de prioridad
    q1 = new_queue(10000);
    q2 = new_queue(10000);
    return *mlfq_scheduler;
  }

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}

