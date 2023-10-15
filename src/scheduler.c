#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "queue.h"

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    int shortest_job_pid = -1;
    int shortest_total_time = -1;

    if(curr_pid != -1)
        return curr_pid;

    for (int i = 0; i < procs_count; i++) {
        int total_time = process_total_time(procs_info[i].pid); // Obtener el tiempo de ejecución
        if (shortest_job_pid == -1 || total_time < shortest_total_time) {
            shortest_job_pid = procs_info[i].pid;
            shortest_total_time = total_time;
        }
    }

    return shortest_job_pid;
}

// Función que devuelve el tiempo restante de ejecución dado un PID
int process_remaining_time(proc_info_t process) {

    return process_total_time(process.pid) - process.executed_time;
}

// Función que implementa la política STCF y devuelve el PID del proceso con el tiempo restante más corto
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    int shortest_time_remaining_pid = -1;
    int shortest_time_remaining = -1;

    for (int i = 0; i < procs_count; i++) {
   
        int time_remaining = process_remaining_time(procs_info[i]); // Obtener el tiempo restante
        if (shortest_time_remaining_pid == -1 || time_remaining < shortest_time_remaining) {
            shortest_time_remaining_pid = procs_info[i].pid;
            shortest_time_remaining = time_remaining;
        }
    }

    return shortest_time_remaining_pid;
}

// Función para inicializar una cola
queue_t *createQueue(unsigned capacity) {
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->length = 0;
    queue->array = (proc_info_t *)malloc(queue->capacity * sizeof(proc_info_t));
    return queue;
}

// Función para encolar un proceso en la cola
void enqueue(queue_t *queue, proc_info_t process) {
    if (queue->rear == queue->capacity - 1) {
        printf("La cola está llena.\n");
        return;
    }
    queue->length++;
    queue->array[++queue->rear] = process;
}

// Función para desencolar un proceso de la cola
proc_info_t dequeue(queue_t *queue) {
    if (queue->front > queue->rear) {
        proc_info_t empty;
        empty.pid = -1;
        return empty;
    }
    queue->length--;
    return queue->array[queue->front++];
}

proc_info_t showFirst(queue_t *queue) {
    if (queue->front > queue->rear) {
        proc_info_t empty;
        empty.pid = -1;
        return empty;
    }
    return queue->array[queue->front];
}

int num_queues;
queue_t *queues;

// Variables globales para el algoritmo Round Robin
const int Time_slice = 4;

int time_slice = Time_slice; // Tamaño del time_slice (puedes ajustarlo según tus necesidades)

const int Time_slice_mlfq = 8;
// Función que implementa la política Round Robin y devuelve el PID del proceso a ejecutar
int rr(queue_t *queue, int procs_count, int curr_pid) {
    
    int last_process_id = curr_pid;
        
    if(curr_pid == -1){ 
        time_slice = Time_slice;
        proc_info_t info = dequeue(queue);
        enqueue(queue, info);
        return info.pid;
    }

    for (int i = 0; i < procs_count; i++) {

        proc_info_t info = dequeue(queue);

        if (info.pid == curr_pid) {

            if (time_slice == 0) {
                time_slice = Time_slice;
                info = dequeue(queue);
                enqueue(queue, info);
                return info.pid;

            } else {
                time_slice = time_slice - 1;
            }
            break;
        }

        enqueue(queue, info);
    }
    
    
    // Devolver el PID del proceso actual
    return last_process_id;
}

int rr2(queue_t *queue, int procs_count, int curr_pid) {
    
    for (int i = 0; i < procs_count; i++) {


        if (time_slice == 0) {
            time_slice = Time_slice;
            // printf("%d----\n", procs_count);
            // printf("%d\n", showFirst(queue).pid);
            proc_info_t info = dequeue(queue);
            
            
            enqueue(queue, info);
            // printf("%d\n", showFirst(queue).pid);
        } else {
            time_slice--;
        }
        break;

    }

    // Devolver el PID del proceso actual
    return showFirst(queue).pid;
}


// Función que implementa la política Round Robin y devuelve el PID del proceso a ejecutar
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    queue_t *queueRR = (queue_t *)malloc(sizeof(queue_t));
    queueRR = createQueue(500);

    for (size_t i = 0; i < procs_count; i++)
    {
        enqueue(queueRR, procs_info[i]);
    }

    int a = rr(queueRR, procs_count, curr_pid);
    // free(queueRR);
    return a;
}


// Variables globales para el algoritmo MLFQ
int last_procs_pid = -1;
int last_last_procs_pid = -1;

const int S = 100;
int s = S;

int toMLFQ = 1;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    if(toMLFQ)
    {
        num_queues = 3;
        queues = (queue_t *)malloc(num_queues * sizeof(queue_t));
        for (int i = 0; i < num_queues; i++) {
            queues[i] = *createQueue(10000); // Capacidad de cada cola
        }
        toMLFQ = 0;
    }

    for (size_t i = procs_count-1; i < -1 ; i--){

        if(procs_info[i].pid != last_procs_pid)
        {
            if(procs_info[i].pid != last_last_procs_pid)
            {
                enqueue(&queues[0],procs_info[i]);
            }
            else
            {
                break;
            }
        }
        else
            break;
    }
    last_procs_pid = procs_info[procs_count-1].pid;
    last_last_procs_pid = procs_info[procs_count-2].pid;


    for (size_t i = 0; i < num_queues; i++)
    {
        int l = queues[i].length;
        for (size_t j = 0; j < l; j++)
        {
            int here = 0;
            proc_info_t info = dequeue(&queues[i]);

            for (size_t k = 0; k < procs_count; k++)
            { 
                if(info.pid == procs_info[k].pid)
                    here = 1;
            }

            if(here)
                enqueue(&queues[i], info);
            
        }
        
    }
    printf("%d, %d, %d\n", queues[0].length, queues[1].length, queues[2].length);


    //Cada cierto tiempo S, todos los procesos se posicionan en la cola de mayor prioridad
    if(s > 0)
        s--;
    else{
        for (size_t i = 1; i < num_queues; i++)
        {
            while (queues[i].front <= queues[i].rear)
            {
                enqueue(&queues[0], dequeue(&queues[i]));
            } 
        }
        
        s = S; 
    }

     
    
    for (int i = 0; i < num_queues; i++) {
        if (queues[i].front <= queues[i].rear) {
            // Devolver el PID del proceso de la cola no vacía con la mayor prioridad
            
            int next = rr2(&queues[i], queues[i].length, -1);

            proc_info_t proc_info = showFirst(&queues[i]);
            
            if(i != num_queues-1)
                changePriorityIf(proc_info, queues[i+1]);
            return proc_info.pid;
        }
    } 
    return -1; // No hay procesos para ejecutar
}

//Una vez el proceso ha consumido un tiempo de ejecucion igual al slice time en un
//nivel determinado se disminuye su prioridad
void changePriorityIf(proc_info_t proc_info, queue_t queue){
    proc_info.time_slice_mlfq++;
    printf("%d\n", proc_info.time_slice_mlfq);
    if(proc_info.time_slice_mlfq == Time_slice_mlfq){
        proc_info.time_slice_mlfq = 0;
        enqueue(&queue, proc_info);
    }
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;
  

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
