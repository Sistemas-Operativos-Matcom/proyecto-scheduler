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


// Variables globales para el algoritmo Round Robin
const int Time_slice = 4;

int time_slice = Time_slice; // Tamaño del time_slice (puedes ajustarlo según tus necesidades)
int current_index = 0; // PID del proceso actual en ejecución

// Función que implementa la política Round Robin y devuelve el PID del proceso a ejecutar
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    int last_process_id = curr_pid;
        
    int current_process_finished = 1;
    for (int i = 0; i < procs_count; i++) {
        if (procs_info[i].pid == curr_pid) {

            current_process_finished = 0;

            if (time_slice == 0) {
                // El proceso actual ha agotado su time_slice, pasar al siguiente proceso
                current_index = (i + 1) % procs_count;
                last_process_id = procs_info[current_index].pid;

                time_slice = Time_slice;
            } else {
                time_slice = time_slice - 1;
            }
            break;
        }
    }
    if(current_process_finished){ 
        time_slice = Time_slice;
        return procs_info[current_index].pid;
    }
    
    // Devolver el PID del proceso actual
    return last_process_id;
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
  

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
