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
  // Calcular el tiempo de ejecución total de todos los procesos
  int total_time = 0;
  for (int i = 0; i < procs_count; i++) {
    total_time += process_total_time(procs_info[i].pid);
  }

  // Calcular el time slice como el tiempo de ejecución total dividido por el número de procesos
  int time_slice = total_time / procs_count;

  // Mantener un índice que apunte al proceso actual
  static int current_index = 0;

  // Obtener el proceso actual
  proc_info_t current_proc = procs_info[current_index];

  // Verificar si el proceso actual ha terminado
  if (process_total_time(current_proc.pid) - current_proc.executed_time  == 0) {
    // Actualizar el índice al siguiente proceso
    current_index = (current_index + 1) % procs_count;
    current_proc = procs_info[current_index];
  }

  // Verificar si el proceso actual ha excedido el time slice
  if (current_proc.executed_time % time_slice == 0 && current_proc.executed_time != 0) {
    // Actualizar el índice al siguiente proceso
    current_index = (current_index + 1) % procs_count;
    current_proc = procs_info[current_index];
  }

  // Actualizar el tiempo ejecutado del proceso actual
  current_proc.executed_time++;

  // Actualizar el tiempo restante de ejecución del proceso actual
  //current_proc.executed_time++;

  // Retornar el PID del proceso actual
  return current_proc.pid;
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

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
