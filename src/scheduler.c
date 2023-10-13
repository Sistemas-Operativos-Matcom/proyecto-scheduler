#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

// Estructura de una cola
struct queue
{
	int *process; 	// array con los pid de los procesos
  int *times; // array con los slide times acumulados de los procesos
	int maxsize;	// capacidad maxima de la cola
	int front;  	// puntero al primer elemento de la cola
	int rear;   	// puntero al ultimo elemento de la cola
	int size;   	// capacidad actual de la cola
};

// Inicializar la cola
struct queue* newQueue(int size)
{
	struct queue *q = NULL;
	q = (struct queue*)malloc(sizeof(struct queue));

	q->process = (int*)malloc(size * sizeof(int));
	q->times = (int*)malloc(size * sizeof(int));
	q->maxsize = size;
	q->front = 0;
	q->rear = -1;
	q->size = 0;

	return q;
}

// Retornar cantidad de procesos en la cola
int size(struct queue *q)
{
	return q->size;
}

// Revisa si la cola está vacía
int isEmpty(struct queue *q) 
{
	return !size(q);
}

// Retorna el pid del primer proceso en la cola
int front_pid(struct queue *q)
{
	if (isEmpty(q))
	{
		printf("Underflow by ask for front_pid\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	return q->process[q->front];
}

// Retorna el tiempo acumulado del primer proceso en la cola
int front_time(struct queue *q)
{
	if (isEmpty(q))
	{
		printf("Underflow by ask for front_time\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	return q->times[q->front];
}

// Actualizar tiempo del proceso
int updateTime(struct queue *q) 
{
  if (isEmpty(q))
	{
		printf("Underflow by updateTime\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	q->times[q->front] += 10;
  return q->times[q->front];
}

// Insertar un proceso en la cola
void push(struct queue *q, int pid, int time)
{
	if (size(q) == q->maxsize)
	{
		printf("Overflow\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

	q->rear = (q->rear + 1) % q->maxsize;	// circular cola
	q->process[q->rear] = pid;
  q->times[q->rear] = time;
	q->size++;

}

// Extraer el primer elemento de la cola
void pop(struct queue *q)
{
	if (isEmpty(q))
	{
		printf("Underflow by pop\nProgram Terminated\n");
		exit(EXIT_FAILURE);
	}

  q->process[q->front]= -1;
  q->times[q->front]= -1;

	q->front = (q->front + 1) % q->maxsize;  // circular cola
	q->size--;

}

// Buscar un pid en la cola de procesos
int existInQueue(struct queue *q, int pid)
{
  for (int i = 0; i < q->size; i++) {
    if (pid == q->process[(q->front + i)%q->maxsize]) {
      return 1;
    }
  }

  return 0;
}

// Colocar los procesos de la cola q2 al inicio de la cola q1 respetando su orden
void transfer_v1(struct queue *q2, struct queue *q1)
{
  int i = 0;
  while (!isEmpty(q2))
  {
    int pid = q2->process[q2->rear];
    q2->rear -= 1;
    q2->size -= 1;
    
    int newfront = (q1->front -1) % q1->maxsize;
    q1->front = newfront;
    q1->size++;
    q1->process[q1->front] = pid;
    q1->times[q1->front] = 0;
    i++;
  }
}

// Colocar los procesos de la cola q2 al final de la cola q1 respetando su orden
void transfer_v2(struct queue *q2, struct queue *q1)
{
  int i = 0;
  while (!isEmpty(q2))
  {
    int pid = front_pid(q2);
    pop(q2);
    push(q1, pid, 0);
  }
  
}

// transfer_v1 arroja mejor turnaround pero peor tiempo de respuesta que transfer_v2


// La función que define un scheduler está compuesta por los siguientes
// parámetros:
//
//  - procs_info: Array que contiene la información de cada proceso activo
//  - procs_count: Cantidad de procesos activos
//  - curr_time: Tiempo actual de la simulación
//  - curr_pid: PID del proceso que se está ejecutando en el CPU
//
// Esta función se ejecuta en cada timer-interrupt donde existan procesos
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


int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  int min = 1e9;

  // Si el proceso no se ha terminado volver a ejecutarlo
  if (curr_pid != -1)
  {
    return curr_pid;
  }

  // Seleccionar el proceso que tenga menor tiempo de duración y ejecutarlo
  int newPid = -1;
  for (int i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) < min)
    {
      min = process_total_time(procs_info[i].pid);
      newPid = procs_info[i].pid;
    }
  }

  return newPid;
  
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{  
  int index_next_process = -1;
  int min_time = 1e9;
  
  // Buscar en cada momento entre todos los procesos disponibles el que menos tiempo le quede
  for (int i = 0; i < procs_count; i++)
  {
    int aux = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;

    if (aux < min_time)
    {
      min_time = aux;
      index_next_process = i;
    }
  }
  
  return procs_info[index_next_process].pid;
}

struct queue *q;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  static int slide_time = 50;

  // Añadir los procesos nuevos a la cola
  for (int i = 0; i < procs_count; i++)
  {
    if (!existInQueue(q, procs_info[i].pid))
    {
      push(q, procs_info[i].pid, 0);
    }
  }

  // Revisar slide time en cola
  if (!isEmpty(q) && front_time(q) >= slide_time ) {
    int pid = front_pid(q);
    pop(q);
    push(q, pid, 0);
  }

  for (int k = 0; k < size(q); k++)
  {
    int pid = front_pid(q);

    // Recorrer la lista de procesos para saber a cual corresponde el primero de la cola
    for (int i = 0; i < procs_count; i++)
    {
      // En esta posición el pid no coincide
      if (procs_info[i].pid != pid)
      {
        // Si se acabaron los procesos y el primero de la cola no ha aparecido significa que este ya se ha finalizado por lo que hay que retirarlo de la cola y analizar el siguiente
        if (i == procs_count - 1)
        {
          pop(q);
          break;
        }
        continue;
      }

      // Si el proceso se encuentra ejecutando io se analiza el siguiente proceso de la cola
      if (procs_info[i].on_io)
      {
        pop(q);
        push(q, pid, 0);
        break;
      }
      else
      {
        updateTime(q);
        return pid;
      }
      
    }

  }

  // En este punto significa que ninguno de los procesos de las colas fue ejecutado
  return -1;
}


struct queue *q1;
struct queue *q2;
int mlfq_scheduler_v1(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  static int slide_time = 50;
  static int priority_boost = 500;
  static int last_pid = -1;

  // Revisar a qué cola perteneció el proceso que finalizó
  if (curr_pid == -1 && last_pid != -1)
  {
    if (!isEmpty(q1) && front_pid(q1) == last_pid && size(q1) > 0)
    {
      pop(q1);
    }
    else
    {
      pop(q2);
    }
  }  
  
  // insertar procesos nuevos en la cola de mayor prioridad
  for (int i = 0; i < procs_count; i++) {
    // asegurar que los procesos no hayan sido ejecutados y que no estén ya en la cola
    if (procs_info[i].executed_time == 0 && !existInQueue(q1, procs_info[i].pid)) {
      push(q1, procs_info[i].pid, 0);
    }
  }

  // si toca el priority boost sube todo los procesos de la cola 2 a la cola 1
  if (curr_time % priority_boost == 0) {
    transfer_v1(q2, q1);
  }

  // Revisar slide time en cola 1
  if (!isEmpty(q1) && front_time(q1) >= slide_time ) {
    int pid = front_pid(q1);
    pop(q1);
    push(q2, pid, 0);
  }

  // Revisar slide time en cola 2
  if (!isEmpty(q2) && front_time(q2) >= slide_time ) {
    int pid2 = front_pid(q2);
    pop(q2);
    push(q2, pid2, 0);
  }

  if (!isEmpty(q1))  // Trabajando en la cola 1
  {
    for (int k = 0; k < size(q1); k++)
    {
      int pid = front_pid(q1);

      // Recorrer la lista de procesos para saber a cual corresponde el primero de la cola 1
      for (int i = 0; i < procs_count; i++)
      {
        // En esta posición el pid no coincide
        if (procs_info[i].pid != pid)
        {
          // Si se acabaron los procesos y el primero de la cola no ha aparecido significa que este ya se ha finalizado por lo que hay que retirarlo de la cola y analizar el siguiente
          if (i == procs_count - 1)
          {
            pop(q1);
            break;
          }
          continue;
        }

        // Si el proceso se encuentra ejecutando io se analiza el siguiente proceso de la cola 1
        if (procs_info[i].on_io)
        {
          pop(q1);
          push(q1, pid, 0);
          break;
        }
        else
        {
          updateTime(q1);
          return pid;
        }
        
      }

    }
  } 
  
  if(!isEmpty(q2))    // Trabajando en la cola 2
  {
    for (int k = 0; k < size(q2); k++)
    {
      int pid = front_pid(q2);

      // Recorrer la lista de procesos para saber a cual corresponde el primero de la cola 2
      for (int i = 0; i < procs_count; i++)
      {
        // En esta posición el pid no coincide
        if (procs_info[i].pid != pid)
        {
          // Si se acabaron los procesos y el primero de la cola no ha aparecido significa que este ya se ha finalizado por lo que hay que retirarlo de la cola y analizar el siguiente
          if (i == procs_count - 1)
          {
            pop(q2);
            break;
          }
          continue;
        }

        // Si el proceso se encuentra ejecutando io se analiza el siguiente proceso de la cola 2
        if (procs_info[i].on_io)
        {
          pop(q2);
          push(q2, pid, 0);
          break;
        }
        else
        {
          updateTime(q2);
          return pid;
        }
        
      }

    }
  }

  // En este punto significa que ninguno de los procesos de las colas fue ejecutado
  return -1;
  
}

int mlfq_scheduler_v2(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  static int slide_time = 50;
  static int priority_boost = 500;
  static int last_pid = -1;

  // Revisar a qué cola perteneció el proceso que finalizó
  if (curr_pid == -1 && last_pid != -1)
  {
    if (!isEmpty(q1) && front_pid(q1) == last_pid && size(q1) > 0)
    {
      pop(q1);
    }
    else
    {
      pop(q2);
    }
  }  
  
  // insertar procesos nuevos en la cola de mayor prioridad
  for (int i = 0; i < procs_count; i++) {
    // asegurar que los procesos no hayan sido ejecutados y que no estén ya en la cola
    if (procs_info[i].executed_time == 0 && !existInQueue(q1, procs_info[i].pid)) {
      push(q1, procs_info[i].pid, 0);
    }
  }

  // si toca el priority boost sube todo los procesos de la cola 2 a la cola 1
  if (curr_time % priority_boost == 0) {
    transfer_v2(q2, q1);
  }

  // Revisar slide time en cola 1
  if (!isEmpty(q1) && front_time(q1) >= slide_time ) {
    int pid = front_pid(q1);
    pop(q1);
    push(q2, pid, 0);
  }

  // Revisar slide time en cola 2
  if (!isEmpty(q2) && front_time(q2) >= slide_time ) {
    int pid2 = front_pid(q2);
    pop(q2);
    push(q2, pid2, 0);
  }

  if (!isEmpty(q1))  // Trabajando en la cola 1
  {
    for (int k = 0; k < size(q1); k++)
    {
      int pid = front_pid(q1);

      // Recorrer la lista de procesos para saber a cual corresponde el primero de la cola 1
      for (int i = 0; i < procs_count; i++)
      {
        if (procs_info[i].pid != pid)
        {
          // Si se acabaron los procesos y el primero de la cola no ha aparecido significa que este ya se ha finalizado por lo que hay que retirarlo de la cola y analizar el siguiente
          if (i == procs_count - 1)
          {
            pop(q1);
            break;
          }
          continue;
        }

        // Si el proceso se encuentra ejecutando io se analiza el siguiente proceso de la cola 1
        if (procs_info[i].on_io)
        {
          pop(q1);
          push(q1, pid, 0);
          break;
        }
        else
        {
          updateTime(q1);
          return pid;
        }
        
      }

    }
  } 
  
  if(!isEmpty(q2))    // Trabajando en la cola 2
  {
    for (int k = 0; k < size(q2); k++)
    {
      int pid = front_pid(q2);

      // Recorrer la lista de procesos para saber a cual corresponde el primero de la cola 2
      for (int i = 0; i < procs_count; i++)
      {
        if (procs_info[i].pid != pid)
        {
          // Si se acabaron los procesos y el primero de la cola no ha aparecido significa que este ya se ha finalizado por lo que hay que retirarlo de la cola y analizar el siguiente
          if (i == procs_count - 1)
          {
            pop(q2);
            break;
          }
          continue;
        }

        // Si el proceso se encuentra ejecutando io se analiza el siguiente proceso de la cola 2
        if (procs_info[i].on_io)
        {
          pop(q2);
          push(q2, pid, 0);
          break;
        }
        else
        {
          updateTime(q2);
          return pid;
        }
        
      }

    }
  }

  // En este punto significa que ninguno de los procesos de las colas fue ejecutado
  return -1;
  
}




// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
