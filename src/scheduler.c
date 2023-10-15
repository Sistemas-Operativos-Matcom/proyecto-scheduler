#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

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

int next_pid = 0;

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                          int curr_pid) {
    // Incrementa next_pid en cada interrupción de tiempo
    int found = 0;
    for (int i = 0; i < procs_count; i++)
    {
      if (procs_info[i].pid == curr_pid)
      {
        found = 1;
      }
    }
    if (found == 1)
    {
      // Devuelve el PID del próximo proceso a ejecutar
      next_pid = (next_pid + 1) % procs_count;
      return procs_info[next_pid].pid;
    }
    if (next_pid >= procs_count)
    {
      return procs_info[0].pid;
    }
      return procs_info[next_pid].pid;

}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  // Inicializamos una variable de tipo proc_info
  proc_info_t shortest_proc;
  int shortest_time = __INT_MAX__; 

  // Recorremos todos los procesos
  for (int i = 0; i < procs_count; i++) {
    // Obtenemos el tiempo total del proceso y su tiempo de ejecución
    int total_time = process_total_time(procs_info[i].pid);

    // Si el proceso no está en I/O y su tiempo restante es menor que el tiempo más corto actual,
    // actualizamos el pid del proceso más corto y su tiempo de ejecución
    if (total_time < shortest_time) {
      shortest_proc = procs_info[i];
      shortest_time = total_time;
    }
  }

  // Devolvemos el pid del proceso más corto
  return shortest_proc.pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    int min_time = __INT_MAX__;
    int selected_pid = -1;

    for (int i = 0; i < procs_count; i++) {
        int remaining_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
        if (remaining_time < min_time) {
            min_time = remaining_time;
            selected_pid = procs_info[i].pid;
        }
    }

    return selected_pid;
}


// #define NUM_QUEUES 3
// #define SLICE_TIME 10
// #define BOOST_TIME 100

// // Estructura para las colas de prioridad
// typedef struct {
//     proc_info_t *procs_info;
//     int procs_count;
// } priority_queue_t;

// priority_queue_t queues[NUM_QUEUES];

// int last_boost_time = 0;

// // Estructura para la información de cada proceso
// typedef struct {
//     int pid; // PID del proceso
//     int on_io; // Indica si el proceso se encuentra realizando una operación IO
//     int executed_time; // Tiempo que el proceso se ha ejecutado (en CPU o en I/O)
//     int accumulated_time; // Tiempo acumulado del proceso en el nivel actual
// } proc_info_t;

// int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
//     // Si ha pasado el tiempo de boost, mueve todos los procesos a la cola de mayor prioridad
//     if (curr_time - last_boost_time >= BOOST_TIME) {
//         for (int i = 0; i < NUM_QUEUES; ++i) {
//             while (queues[i].procs_count > 0) {
//                 int pid = get_next_pid(&queues[i]);
//                 remove_from_queue(&queues[i], pid);
//                 add_to_queue(&queues[NUM_QUEUES - 1], pid);
//             }
//         }
//         last_boost_time = curr_time;
//     }

//     // Añade el proceso actual a la cola correspondiente
//     if (curr_pid != -1) {
//         int curr_priority = get_priority(curr_pid);
//         add_to_queue(&queues[curr_priority], curr_pid);
//     }

//     // Busca el proceso con mayor prioridad que pueda ejecutarse
//     for (int i = NUM_QUEUES - 1; i >= 0; --i) {
//         if (queues[i].procs_count > 0) {
//             int next_pid = get_next_pid(&queues[i]);

//             // Si el proceso ha consumido su slice time, disminuye su prioridad
//             if (procs_info[next_pid].accumulated_time >= SLICE_TIME) {
//                 if (i > 0) {
//                     remove_from_queue(&queues[i], next_pid);
//                     add_to_queue(&queues[i - 1], next_pid);
//                 }
//                 procs_info[next_pid].accumulated_time = 0;
//             }

//             return next_pid;
//         }
//     }

//     // Si no hay procesos que puedan ejecutarse, devuelve -1
//     return -1;
// }
// // Devuelve la prioridad de un proceso dado su pid
// int get_priority(int pid) {
//     for (int i = NUM_QUEUES - 1; i >= 0; --i) {
//         for (int j = 0; j < queues[i].procs_count; ++j) {
//             if (queues[i].procs_info[j].pid == pid) {
//                 return i;
//             }
//         }
//     }
//     return -1;
// }

// // Devuelve el pid del próximo proceso que se va a ejecutar en una cola dada
// int get_next_pid(priority_queue_t *queue) {
//     static int last_index[NUM_QUEUES] = {0};
//     int queue_index = queue - queues; // Obtiene el índice de la cola
//     int next_index = last_index[queue_index];
//     if (next_index >= queue->procs_count) {
//         next_index = 0;
//     }
//     last_index[queue_index] = next_index + 1;
//     return queue->procs_info[next_index].pid;
// }

// // Añade un proceso a una cola dada
// void add_to_queue(priority_queue_t *queue, int pid) {
//     int new_count = queue->procs_count + 1;
//     queue->procs_info = realloc(queue->procs_info, new_count * sizeof(proc_info_t));
//     queue->procs_info[new_count - 1].pid = pid;
//     queue->procs_count = new_count;
// }

// // Elimina un proceso de una cola dada
// void remove_from_queue(priority_queue_t *queue, int pid) {
//     int index = -1;
//     for (int i = 0; i < queue->procs_count; ++i) {
//         if (queue->procs_info[i].pid == pid) {
//             index = i;
//             break;
//         }
//     }
//     if (index != -1) {
//         for (int i = index + 1; i < queue->procs_count; ++i) {
//             queue->procs_info[i - 1] = queue->procs_info[i];
//         }
//         int new_count = queue->procs_count - 1;
//         queue->procs_info = realloc(queue->procs_info, new_count * sizeof(proc_info_t));
//         queue->procs_count = new_count;
//     }
// }





// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "rr") == 0) return *round_robin_scheduler;
  // if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
