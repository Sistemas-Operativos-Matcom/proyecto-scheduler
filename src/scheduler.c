#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "simulation.h"
#include "queue.h"

struct queue *queue1;
struct queue *queue2;
struct queue *queue3;
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

int shortest_job_first_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Si ya hay un proceso en ejecución, no se realizan cambios
  if (curr_pid != -1) return curr_pid;

  // Inicializamos las variables que llevarán el registro del proceso con el tiempo más corto.
  int shortest_job_time = INT_MAX;  // Tiempo más corto inicialmente se establece como el máximo entero posible.
  int shortest_job_pid = procs_info[0].pid; // Se asume inicialmente que el primer proceso tiene el tiempo más corto.

  for (int i = 0; i < procs_count; i++) {
    int pid = procs_info[i].pid; // Obtenemos el PID del proceso actual.
    int total_time = process_total_time(pid); // Obtenemos el tiempo total de ejecución del proceso.

    // Comparamos el tiempo total del proceso actual con el tiempo más corto registrado hasta ahora.
    if (total_time < shortest_job_time) {
      shortest_job_pid = pid; // Actualizamos el PID del proceso más corto.
      shortest_job_time = total_time; // Actualizamos el tiempo más corto.
    }
  }

  // Retornamos el PID del proceso más corto encontrado.
  return shortest_job_pid;
}

int shortest_time_to_completion_first_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    int shortest_job_pid = -1;      // Almacena el PID del proceso más corto inicialmente configurado como -1
    int shortest_job_time = INT_MAX; // Almacena el tiempo restante del proceso más corto inicializado como el valor máximo de un entero

    // Itera a través de los procesos activos
    for (int i = 0; i < procs_count; i++) {
        int pid = procs_info[i].pid;               // Obtiene el PID del proceso actual
        int executed_time = procs_info[i].executed_time; // Obtiene el tiempo de ejecución ya realizado por el proceso
        int total_time = process_total_time(pid);   // Obtiene el tiempo total requerido para el proceso

        int remaining_time = total_time - executed_time; // Calcula el tiempo restante para el proceso actual

        // Comprueba si el tiempo restante del proceso actual es menor que el tiempo del proceso más corto encontrado hasta ahora
        if (remaining_time < shortest_job_time) {
            shortest_job_pid = pid;            // Actualiza el PID del proceso más corto
            shortest_job_time = remaining_time; // Actualiza el tiempo del proceso más corto
        }
    }

    return shortest_job_pid; // Devuelve el PID del proceso más corto encontrado
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  // Declaración de variables estáticas para mantener el estado entre llamadas.
  static int slice = 1;         // Variable que cuenta el tiempo transcurrido.
  static int current_index = 0; // Índice del proceso actual en el arreglo procs_info.

  // Incrementa el contador de tiempo (slice) en cada llamada al planificador.
  if (!((++slice) % 10)) {
    // Cuando se alcanza un múltiplo de 10 (cada 10 unidades de tiempo),
    // se cambia al siguiente proceso en la cola de procesos listos.
    current_index = (current_index + 1) % procs_count;
    // Devuelve el PID (identificador de proceso) del próximo proceso a ejecutar.
    return procs_info[current_index].pid;
  }

  // Si no se alcanza un múltiplo de 10, el proceso actual continúa ejecutándose.
  // Devuelve el PID del proceso actual.
  return curr_pid;
}

int my_rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
    // Variables para llevar el control del próximo proceso a ejecutar y del quantum
    static int next_process_index = 0;
    static int quantum = 10;  // Puedes ajustar el valor del quantum según tus necesidades

    // Obtener información del proceso actual
    int current_index = -1;
    for (int i = 0; i < procs_count; i++) {
        if (procs_info[i].pid == curr_pid) {
            current_index = i;
            break;
        }
    }

    // Verificar si el proceso actual ha agotado su quantum
    if (current_index >= 0 && procs_info[current_index].executed_time >= quantum) {
        // Reiniciar el quantum del proceso actual
        procs_info[current_index].executed_time = 0;
        
        // Pasar al siguiente proceso en la lista circular
        next_process_index = (current_index + 1) % procs_count;
    }

    // Encontrar el próximo proceso listo para ejecutar
    int next_index = -1;
    for (int i = 0; i < procs_count; i++) {
        int index = (next_process_index + i) % procs_count;
        if (!procs_info[index].on_io) {
            next_index = index;
            break;
        }
    }

    if (next_index != -1) {
        // Actualizar el índice del próximo proceso a ejecutar
        next_process_index = (next_index + 1) % procs_count;
        
        // Devolver el PID del próximo proceso a ejecutar
        return procs_info[next_index].pid;
    } else {
        // No hay procesos listos para ejecutar en este momento
        return -1;
    }
}


int multi_level_feedback_queue(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    static int boost = 500;      // Valor de aumento para prioridad
    static int time = 60;        // Tiempo para cambiar de cola
    static int lpid = -1;        // PID del último proceso ejecutado

    // Verificar si el proceso actual debe ser desplazado de una cola a otra
   if (curr_pid == -1 && lpid != -1) {
        if (!e_queue(queue1) && f_pid(queue1) == lpid && size(queue1) > 0) {
            pop(queue1);  // Eliminar de cola 1 si está presente
        } else if (!e_queue(queue2) && f_pid(queue2) == lpid && size(queue2) > 0) {
            pop(queue2);  // Eliminar de cola 2 si está presente
        } else if (!e_queue(queue3) && f_pid(queue3) == lpid && size(queue3) > 0) {
            pop(queue3);  // Eliminar de cola 3 si está presente
        }
    }

    // Agregar procesos nuevos a la cola 1
    for (int i = 0; i < procs_count; i++) {
        if (procs_info[i].executed_time == 0 && !PidQueue(queue1, procs_info[i].pid)) {
            push(queue1, 0, procs_info[i].pid);
        }
    }
    
    // Realizar un "boost" para mover procesos de cola 2 a cola 1 y de la cola 3 a la 2 en intervalos regulares
    if (curr_time % boost == 0) {
        queue2toqueue1(queue2, queue1);
        queue2toqueue1(queue3, queue2);
    }

    // Comprobar si un proceso en cola 1 ha estado en ejecución durante "time" unidades de tiempo
    if (!e_queue(queue1) && f_time(queue1) >= time) {
        int pid = f_pid(queue1);
        pop(queue1);
        push(queue2, 0, pid);  // Mover a cola 2
    }

    // Comprobar lo mismo para cola 2
    if (!e_queue(queue2) && f_time(queue2) >= time) {
        int pid2 = f_pid(queue2);
        pop(queue2);
        push(queue3, 0, pid2);  // Mover a cola 2 nuevamente
    }
    // Comprobar lo mismo para cola 3
    if (!e_queue(queue3) && f_time(queue3) >= time) {
        int pid = f_pid(queue3);
        pop(queue3);
        push(queue3, 0, pid);  // Mover a cola 3 nuevamente
    }

    // Ejecutar procesos en cola 1
    if (!e_queue(queue1)) {
        int pidToExecute = executeProcessesInQueue(queue1, procs_info, procs_count);
        if (pidToExecute != -1) {
            return pidToExecute;
        }
    }

    // Ejecutar procesos en cola 2
    if (!e_queue(queue2)) {
       int pidToExecute = executeProcessesInQueue(queue2, procs_info, procs_count);
        if (pidToExecute != -1) {
            return pidToExecute;
        }
    }
    
      // Ejecutar procesos en cola 3
      if (!e_queue(queue3)) {
    int pidToExecute = executeProcessesInQueue(queue3, procs_info, procs_count);
    if (pidToExecute != -1) {
        return pidToExecute;
    }
  }
      return -1;
     // Si no se selecciona ningún proceso para ejecución
  }
int executeProcessesInQueue(struct queue* queue1, proc_info_t *procs_info, int procs_count) {
    for (int i = 0; i < size(queue1); i++) {
        int pid = f_pid(queue1);
        for (int j = 0; j < procs_count; j++) {
            if (procs_info[j].pid != pid) {
                if (j == procs_count - 1) {
                    pop(queue1);
                    break;
                }
                continue;
            }
            if (procs_info[j].on_io) {
                pop(queue1);
                push(queue1, 0, pid);  // Mover a cola posterior si está en E/S
                break;
            } else {
                u_time(queue1);  // Actualizar tiempo en cola
                return pid;      // Devolver el PID para ejecución
            }
        }
    }
    return -1; // Si no se selecciona ningún proceso para ejecución
}


// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *shortest_job_first_scheduler;
  if (strcmp(name, "stcf")== 0) return *shortest_time_to_completion_first_scheduler;
  if (strcmp(name, "rr") == 0) return *round_robin_scheduler;
  if (strcmp(name, "rrm") == 0) return *my_rr_scheduler;
  if (strcmp(name, "mlfq") == 0) 
  { queue1 = createQueue(1000);
    queue2 = createQueue(1000);
    queue3 = createQueue(1000);
    return *multi_level_feedback_queue;}
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
