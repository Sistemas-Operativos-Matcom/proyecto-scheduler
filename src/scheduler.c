#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include <limits.h>

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
  // Ordenar los procesos por su tiempo de duración
  for (int i = 0; i < procs_count - 1; i++) {
    for (int j = i + 1; j < procs_count; j++) {
      if (process_total_time(procs_info[i].pid) > process_total_time(procs_info[j].pid)) {
        proc_info_t temp = procs_info[i];
        procs_info[i] = procs_info[j];
        procs_info[j] = temp;
      }
    }
  }

  // Retornar el PID del proceso con el menor tiempo de duración
  return procs_info[0].pid;
}
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  // Ordenar los procesos por su tiempo de ejecución restante
  for (int i = 0; i < procs_count - 1; i++) {
    for (int j = i + 1; j < procs_count; j++) {
      if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time > process_total_time(procs_info[j].pid) - procs_info[j].executed_time) {
        proc_info_t temp = procs_info[i];
        procs_info[i] = procs_info[j];
        procs_info[j] = temp;
      }
    }
  }

  // Retornar el PID del proceso con el menor tiempo de ejecución restante
  return procs_info[0].pid;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  static int current_process = 0;
  static int time_slice = 10; // Time slice of 10 units

  // Check if the current process has finished executing
  if (procs_info[current_process].executed_time == process_total_time(procs_info[current_process].pid)) {
    current_process = (current_process + 1) % procs_count; // Move to the next process
    time_slice = 10; // Reset the time slice
  }

  // Check if the current process has exceeded its time slice
  if (time_slice == 0) {
    current_process = (current_process + 1) % procs_count; // Move to the next process
    time_slice = 10; // Reset the time slice
  }

  // Execute the current process for one unit of time
  int pid = procs_info[current_process].pid;
  //procs_info[current_process].executed_time++;
  time_slice--;

  return pid;
}


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


  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
