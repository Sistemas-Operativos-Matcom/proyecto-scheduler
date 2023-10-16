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
                     int curr_pid){
    int min_time = 2147483647;
    int selected_pid = -1;

    for (int i = 0; i < procs_count; i++) {
    
      int p_duration= process_total_time(procs_info[i].pid);

        if (p_duration < min_time) {
            min_time = p_duration;
            selected_pid = procs_info[i].pid;
        }
    }
    return selected_pid;

}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid){
int min_time = 2147483647;
    int selected_pid = -1;

      for (int i = 0; i < procs_count; i++) {
      int p_duration= process_total_time(procs_info[i].pid);
     int exec_time = procs_info[i].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)
    if(p_duration-exec_time<min_time){
      min_time=p_duration-exec_time;
      selected_pid=procs_info[i].pid;
    }
       
    }

    return selected_pid;

}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  static const int SLICING_FACTOR = 5;
  static int time_slice_cont = 0;  
  static int proc_ind = 0;

  if (time_slice_cont<SLICING_FACTOR && curr_pid!=-1){
    time_slice_cont++;
    return curr_pid;
  }
  
  proc_ind=(proc_ind+1) % procs_count;
  time_slice_cont = 0;

  return procs_info[proc_ind].pid;
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
  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
