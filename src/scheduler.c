#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "process.h"
#include "simulation.h"

static int find_proc_info_index_by_pid(proc_info_t *procs_info, int procs_count, int pid) {
  for (int i = 0; i < procs_count; i++) {
    if (procs_info[i].pid == pid) return i;
  }
  return -1;
}

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
//   int exec_time =
//       procs_info[0].executed_time;  // Tiempo que lleva el proceso activo
//                                     // (curr_time - arrival_time)

//   // También puedes usar funciones definidas en `simulation.h` para extraer
//   // información extra:
//   int duration = process_total_time(pid);

//   return -1;
// }

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  if (procs_count == 0) return -1;

  if (find_proc_info_index_by_pid(procs_info, procs_count, curr_pid) != -1) return curr_pid;

  int less_duration = INT_MAX;
  int less_duration_pid = -1;

  for (int i = 0; i < procs_count; i++) {
    int duration = process_total_time(procs_info[i].pid);

    if (duration < less_duration) {
      less_duration = duration;
      less_duration_pid = procs_info[i].pid;
    }
  }

  return less_duration_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  if (procs_count == 0) return -1;

  int less_duration = INT_MAX;
  int less_duration_pid = -1;

  for (int i = 0; i < procs_count; i++) {
    int remain_duration = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;

    if (remain_duration < less_duration) {
      less_duration = remain_duration;
      less_duration_pid = procs_info[i].pid;
    }
  }

  return less_duration_pid;  
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  if (procs_count == 0) return -1;
  if (curr_pid == -1) return procs_info[0].pid;
  
  if (curr_time % ROUND_ROBIN_TIME_SLICE != 0) {    
    return curr_pid;
  }

  for (int i = 0; i < procs_count; i++) {
    if (procs_info[i].pid == curr_pid) {
      int start_index = ++i;
      while (1) {
        if (!procs_info[i % procs_count].on_io)
          break;
        i++;
        if (i == start_index) {
          break;
        }
      }
      return procs_info[i % procs_count].pid;
    }
  }
}
// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "round_robin") == 0) return *rr_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
