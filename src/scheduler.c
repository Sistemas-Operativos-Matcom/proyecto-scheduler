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

// Estrategia Shortest Job First(SJF)
int SJF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
  int sj_pid = procs_info[0].pid;
  int sj_duration = process_total_time(sj_pid);

  for(int i = 1; i < procs_count; i++)
  {
    int pid = procs_info[i].pid;
    int duration = process_total_time(pid);

    if(sj_duration > duration)
    {
      sj_duration = duration;
      sj_pid = pid;
    }
  }
  return sj_pid;
}

//Estrategia Shortest Time-to-Completion First(STCF)
int STCF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int stc_pid = procs_info[0].pid;
  int stc_left_time = process_total_time(stc_pid) - procs_info[0].executed_time;

  for(int i = 1; i < procs_count; i++)
  {
    int pid = procs_info[i].pid;
    int left_time = process_total_time(pid) - procs_info[i].executed_time;

    if(stc_left_time > left_time)
    {
      stc_left_time = left_time;
      stc_pid = pid;
    }
  }
  return stc_pid;
}

//Estrategia Round Robin(RR)
int RR_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // time slice = timer interrupt * 3
  static int tsc = 0; // contador para el time slice
  static int process_i = 0;
  
  if(curr_pid != -1 && tsc < 3)
  {
    tsc ++;
    return curr_pid;
  }

  tsc = 0;
  process_i = (process_i + 1) % procs_count;
  return procs_info[process_i].pid;
}

int MLFQ_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  return -1;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *SJF_scheduler;
  if (strcmp(name, "stcf") == 0) return *STCF_scheduler;
  if (strcmp(name, "rr") == 0) return *RR_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}

//int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
//int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)