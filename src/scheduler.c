#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "structures.h"
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


int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  if(curr_pid==-1)
  {
    int min = 0;
    for(int i=0; i<procs_count;i++)
    {
      min = (process_total_time(procs_info[min].pid)<process_total_time(procs_info[i].pid))?min:i;
    }
    return procs_info[min].pid;
  }
  return curr_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  int min = 0;
  for(int i=0; i<procs_count;i++)
  {
    int temp1 = process_total_time(procs_info[min].pid)-procs_info[min].executed_time;
    int temp2 = process_total_time(procs_info[i].pid)-procs_info[i].executed_time;
    min = (temp1<temp2)?min:i;
  }
  return procs_info[min].pid;
}


int round_time;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  static int round_length = 0;
  if(round_length == 0 || curr_pid == -1)
  {
    static int current_proc = -1;
    current_proc+=1;
    round_length = 5;
    return procs_info[current_proc%procs_count].pid;
  }
  round_length-=1;
  return curr_pid;
}

queue_t *q1;
queue_t *q2;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  // hola!
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
  if (strcmp(name, "mlfq") == 0)
  {
    q1 = build_queue();
    q2 = build_queue();
    return *mlfq_scheduler;
  }  


  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
