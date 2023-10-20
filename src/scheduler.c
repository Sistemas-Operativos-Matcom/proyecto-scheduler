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
  static int proc_ind = 0;
  static const int slice_time = 50;

  // counting processes not on I/O cuchauuuuu
    int io_proc_cont = 0;
    for (int i = 0; i < procs_count; i++) {

        if (!procs_info[i].on_io) {

            io_proc_cont++;
        }
    }

    if (io_proc_cont == 0) {
      return -1;
    } 

    proc_info_t io_proc_data [io_proc_cont];
    int idx = 0;

    //filtering processes not on I/O
    for (int i = 0; i < procs_count; i++) {
        if (!procs_info[i].on_io) {
            io_proc_data[idx++] = procs_info[i];
        }
    }
  
   
  if (curr_time%slice_time==0){proc_ind++;}

  proc_ind =proc_ind% io_proc_cont;

  return io_proc_data[proc_ind].pid;
}

 int boost_cooldown= 100;
 int max_cpu_time = 50;
 int canon [100000];    // canon denotes the relevance of a process, whether is canon or not xd
 int cur_cpu_time [100000];

// Multi-level Feedback Queue
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {

    // priority and time boost loop
    for (int i = 0; i < procs_count; i++) {
        int proc_pid = procs_info[i].pid;

        if (!procs_info[i].on_io) {
            cur_cpu_time[proc_pid] += 10; 
        }

        if (max_cpu_time <= cur_cpu_time[proc_pid] ) {
            cur_cpu_time[proc_pid] = 0;
            canon[proc_pid]++;
        }
    }

    // resetting priority and CPU time
    if (curr_time % boost_cooldown == 0) {
        for (int i = 0; i < procs_count; i++) {
            int proc_pid = procs_info[i].pid;
            canon[proc_pid] = 0;

            cur_cpu_time[proc_pid] = 0;
        }
    }

    // counting processes not on I/O cuchauuuuu
    int io_proc_cont = 0;
    for (int i = 0; i < procs_count; i++) {

        if (!procs_info[i].on_io) {
            io_proc_cont++;
        }
    }

    if (io_proc_cont == 0) {
      return -1;
    } 

    proc_info_t io_proc_data [io_proc_cont];
    int idx = 0;

    //filtering processes not on I/O
    for (int i = 0; i < procs_count; i++) {
        if (!procs_info[i].on_io) {
            io_proc_data[idx++] = procs_info[i];
        }
    }

    int canon_cont = 0;

    int min_priority = 1000000000;

    for (int i = 0; i < io_proc_cont; i++) {

        int proc_pid = io_proc_data[i].pid;

        if (canon[proc_pid] < min_priority) {
            canon_cont = 0;
            min_priority = canon[proc_pid];
        }
         if (canon[proc_pid] == min_priority) {
            canon_cont++;
        }
    }

    proc_info_t top_proc[canon_cont];    // here there will be stored the most canon events, the next to process
    int cp_idx = 0;       // canon process index

    //extracting high priority processes
    for (int i = 0; i < io_proc_cont ; i++) {
        int proc_pid = io_proc_data[i].pid;

        if (canon[proc_pid] == min_priority) {
            top_proc[cp_idx++] = io_proc_data[i];
        }
    }

    return rr_scheduler(top_proc, canon_cont, curr_time, curr_pid);
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
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;
  

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
