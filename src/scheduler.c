#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int *act_proc=-1;


int check_PID(proc_info_t *procs_info, int count, int act_proc_PID){

for (size_t i = 0; i < count; i++)
{
  if(procs_info[i].pid==act_proc_PID){
    return 1;
  }
}

return 0;

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

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time =
      procs_info[0].executed_time;  // Tiempo que lleva el proceso activo
                                    // (curr_time - arrival_time)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}

int sjf(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid){

if (check_PID(procs_info,procs_count,act_proc))
{
  return act_proc;
}

proc_info_t *procs_info_copy =procs_info;

       for (size_t i = 0; i < procs_count; i++)
       {
        for (size_t j = i+1; i < procs_count; j++)
        {
          if( process_total_time(procs_info_copy[i].pid)> process_total_time(procs_info_copy[j].pid)){
            proc_info_t swap = procs_info_copy[i];
            procs_info_copy[i]=procs_info_copy[j];
            procs_info_copy[i]=swap;
          }
        }
        
       }

       act_proc=procs_info_copy[0].pid;
       return act_proc;


}


int stcf(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid){

       proc_info_t *procs_info_copy =procs_info;

       for (size_t i = 0; i < procs_count; i++)
       {
        for (size_t j = i+1; i < procs_count; j++)
        {
          if( process_total_time(procs_info_copy[i].pid)> process_total_time(procs_info_copy[j].pid)){
            proc_info_t swap = procs_info_copy[i];
            procs_info_copy[i]=procs_info_copy[j];
            procs_info_copy[i]=swap;
          }
        }
        
       }

       return procs_info_copy[0].pid;
           
}

int rr(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid){



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
