#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int time_slice = 0;
int preview_pid = 0;

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
                     int curr_pid) 
{
  
  int min = process_total_time(procs_info[0].pid);
  int pid_min = procs_info[0].pid;

    for(int i = 1; i < procs_count; i++)
    {
         if(process_total_time(procs_info[i].pid) <= min)
         {
         pid_min = procs_info[i].pid;
         }
    }
  
     return pid_min;
}
  
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  int min = process_total_time(procs_info[0].pid);
  int pid_min = procs_info[0].pid;
  
    for(int i = 1; i < procs_count; i++)
    {
      
         if((process_total_time(procs_info[0].pid)-procs_info[i].executed_time)<min)
         pid_min = i;
    }
  
  
  return pid_min;
  
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

  if(time_slice == 5 )
  {
    time_slice = 0;
    if(curr_pid == -1)
    {
      if (preview_pid >= procs_count)
      {
        preview_pid = 0;
      return procs_info[0].pid;    
      }
      return procs_info[preview_pid].pid;  
    }
    else
    {
      preview_pid++;
      if(preview_pid>=procs_count)
      preview_pid=0;
      return procs_info[preview_pid].pid;
    }
  }
  else
  {
    time_slice++;
    if(curr_pid == -1)
    {
      time_slice = 0;

     if (preview_pid >= procs_count)
      {
        preview_pid = 0;
      return procs_info[0].pid;    
      }
      return procs_info[preview_pid].pid;  
    }
    else
    {
      return curr_pid;
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
   if (strcmp(name, "rr") == 0) return *rr_scheduler;


  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
