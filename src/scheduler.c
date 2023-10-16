#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "queue.h"

int num_queues;
queue_t *queues;
queue_t queueRR;

// Variables globales para el algoritmo Round Robin
const int Time_slice = 3;

int time_slice = Time_slice; // Tamaño del time_slice (puedes ajustarlo según tus necesidades)

const int Time_slice_mlfq = 6;

// Variables globales para el algoritmo MLFQ
int last_procs_pid = -1;
int last_last_procs_pid = -1;

const int S = 50;
int s = S;

int toMLFQ = 1;
int toRR = 1;

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
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

//Round Robin con una cola
int rr(queue_t *queue) {
    
    if (time_slice == 0) {
        time_slice = Time_slice;          
        enqueue(queue, dequeue(queue));
    } else {
        time_slice--;
    }
    return queue->array[queue->front].pid;
}

// Función que implementa la política Round Robin y devuelve el PID del proceso a ejecutar
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    //Inicializa la cola
    if(toRR){
        queueRR = *createQueue(1000000);
        toRR = 0;
    }

    //Guarda en la cola solo los nuevos procesos
    for (size_t i = procs_count-1; i < -1 ; i--){

        if(procs_info[i].pid != last_procs_pid)
        {
            if(procs_info[i].pid != last_last_procs_pid)
                enqueue(&queueRR,procs_info[i]);
            else
                break;
        }
        else
            break;
    }
    last_procs_pid = procs_info[procs_count-1].pid;
    last_last_procs_pid = procs_info[procs_count-2].pid;

    //Limpia la cola
    int l = queueRR.length;
    for (size_t j = 0; j < l; j++)
    {
        int here = 0;
        for (size_t k = 0; k < procs_count; k++)
        { 
            if(queueRR.array[queueRR.front].pid == procs_info[k].pid)
                here = 1;
        }
        if(here)
            enqueue(&queueRR, dequeue(&queueRR));
        else{
            dequeue(&queueRR);
            time_slice = Time_slice;
        }
    }

    int a = rr(&queueRR);
    return a;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    
    //Reserva memoria para las colas
    if(toMLFQ)
    {
        num_queues = 3;
        queues = (queue_t *)malloc(num_queues * sizeof(queue_t));
        for (int i = 0; i < num_queues; i++) {
            queues[i] = *createQueue(1000000); // Capacidad de cada cola
        }
        toMLFQ = 0;
    }

    //Guarda en la cola los nuevos procesos
    for (size_t i = procs_count-1; i < -1 ; i--){

        if(procs_info[i].pid != last_procs_pid)         //Revisa de atras hacia delante, buscando el
        {                                               //ultimo o el penultimo proceso que conocia
            if(procs_info[i].pid != last_last_procs_pid)
            {
                enqueue(&queues[0],procs_info[i]);
                printf("Entra el proceso %d en la cola 0\n", procs_info[i].pid);
            }
            else
                break;
        }
        else
            break;
    }
    last_procs_pid = procs_info[procs_count-1].pid;
    last_last_procs_pid = procs_info[procs_count-2].pid;

    //Limpia la cola
    for (size_t i = 0; i < num_queues; i++)
    {
        int l = queues[i].length;
        for (size_t j = 0; j < l; j++)
        {
            int here = 0;//booleano para saber si un proceso sigue activo

            for (size_t k = 0; k < procs_count; k++)
            { 
                if(queues[i].array[queues[i].front].pid == procs_info[k].pid)
                    here = 1;
            }
            if(here)
                enqueue(&queues[i], dequeue(&queues[i]));
            else
                dequeue(&queues[i]);//Si ya no esta activo lo saco de las colas
        }
        
    }

    //Cada cierto tiempo S, todos los procesos se posicionan en la cola de mayor prioridad
    if(s > 0)
        s--;
    else{
        for (size_t i = 1; i < num_queues; i++)
        {
            while (queues[i].front <= queues[i].rear)
                enqueue(&queues[0], dequeue(&queues[i]));
        }
        printf("Todos los procesos vuelven a la mayor prioridad\n");
        printf("Estados de las colas %d, %d, %d\n", queues[0].length, queues[1].length, queues[2].length);
        s = S; 
    }

    //Pr(A) > Pr(B)  => Se ejecuta A
    for (int i = 0; i < num_queues; i++) {
        if (queues[i].front <= queues[i].rear) {//Si la cola no esta vacia

            //Dos procesos con igual prioridad se deciden por RR
            rr(&queues[i]);

            if(i != num_queues-1) //Solo modifica su prioridad por Time Slice si no es la ultima prioridad
            {
                queues[i].array[queues[i].front].time_slice_mlfq += 1;

                //Si alcanzo el time Slice
                if( queues[i].array[queues[i].front].time_slice_mlfq == Time_slice_mlfq ){

                    queues[i].array[queues[i].front].time_slice_mlfq = 0;
                    enqueue(&queues[i+1], dequeue(&queues[i]));
                    printf("El proceso %d entra a la cola %d\n", queues[i+1].array[queues[i+1].rear].pid, i+1);
                    printf("Estados de las colas %d, %d, %d\n", queues[0].length, queues[1].length, queues[2].length);

                    if (queues[i].front > queues[i].rear)//Si se saco el unico elemento de la cola ve a otra
                        continue;
                }
            }
            printf("Se ejecuta el proceso %d de la cola %d\n", queues[i].array[queues[i].front].pid, i);
            return queues[i].array[queues[i].front].pid;
        }
    } 
    return -1; // No hay procesos para ejecutar
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
