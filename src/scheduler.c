#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include <limits.h>
//#include "queue.c"

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

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  static int index = 0;
  int new_curr_pid = -1;
  int temp = INT_MAX;
  if (process_total_time(procs_info[index].pid) - procs_info[index].executed_time == 0){
    for (int i = 0; i < procs_count; i++) {
      if (process_total_time(procs_info[i].pid) < temp){

        temp = process_total_time(procs_info[i].pid);

        new_curr_pid = procs_info[i].pid;

      }
    }
  }

  return new_curr_pid;
}
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int new_curr_pid = curr_pid;
  int min_remaining_time = INT_MAX;
  
  for (int i = 0; i < procs_count; i++) {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < min_remaining_time) {
      min_remaining_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      new_curr_pid = procs_info[i].pid;
    }
  }

  return new_curr_pid;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
    
    static int last_index = 0;
    int next_pid = -1;

    if (curr_pid == -1) {
      next_pid = procs_info[last_index].pid;    
    }
    else if(last_index + 1 == procs_count) {
      next_pid = procs_info[0].pid;
      last_index = 0;
    }
    else {
      next_pid = procs_info[last_index + 1].pid;
      last_index += 1 ;
    }
    
   
    return next_pid;
    
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
    
    int new_curr_pid = curr_pid;
    int min_executed_time = INT_MAX;
  
    for (int i = 0; i < procs_count; i++) {
      if (procs_info[i].executed_time < min_executed_time && procs_info[i].pid != curr_pid){ 
        min_executed_time = procs_info[i].executed_time;
        new_curr_pid = procs_info[i].pid;
      }
    }
  
    return new_curr_pid;
}

/*#define MAX_PROCESSES 100

typedef struct {
  int pid; // ID del proceso
  int priority; // Prioridad del proceso
  int executed_time; // Tiempo que el proceso se ha ejecutado
} Process;

Queue *queues[3]; // Tres colas con diferentes niveles de prioridad
Process processes[MAX_PROCESSES]; // Arreglo que contiene los procesos
int num_processes = 0; // Número de procesos en el arreglo

// Función para agregar un proceso a la cola de nivel más bajo
void add_process(Process process) {
  enqueue(queues[0], num_processes);
  processes[num_processes] = process;
  num_processes++;
}

// Función para ejecutar la estrategia MLFQ
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  static Queue *queues[3];
  static int current_process = 0;
  // Verificar si el proceso actual ha terminado
  if (procs_info[current_process].executed_time == process_total_time(procs_info[current_process].pid)) {
    dequeue(queues[procs_info[current_process].on_io]); // Eliminar el proceso de la cola actual
    return -1; // Indicar que se necesita un nuevo proceso
  }

  // Verificar si el proceso actual ha excedido su tiempo límite
  if (curr_time % 10 == 0 && curr_time != 0) {
    if (procs_info[current_process].on_io < 2) {
      procs_info[current_process].on_io++; // Incrementar la prioridad del proceso
      dequeue(queues[procs_info[current_process].on_io - 1]); // Eliminar el proceso de la cola actual
      enqueue(queues[procs_info[current_process].on_io], curr_pid); // Agregar el proceso a la cola de prioridad más alta
    }
  }

  // Obtener el siguiente proceso a ejecutar
  int next_pid = -1;
  for (int i = 0; i < 3; i++) {
    if (!is_empty(queues[i])) {
      next_pid = dequeue(queues[i]);
      break;
    }
  }

  // Si no hay procesos en las colas, continuar con el proceso actual
  if (next_pid == -1) {
    return curr_pid;
  }

  // Ejecutar el siguiente proceso
  //processes[next_pid].executed_time++;
  return next_pid;
}*/




int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "round_robin") == 0) return *round_robin_scheduler;
  //if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;


  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
