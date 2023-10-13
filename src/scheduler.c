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

void io_remover(proc_info_t *procs_info, int *procs_count)
{
  int procs_not_on_io_count = 0;
  proc_info_t *procs_not_on_io_info = malloc(sizeof(proc_info_t) * (*procs_count));
  int proc_index = 0;

  for(int c=0; c<(*procs_count); c++)
  {
    if(!procs_info[c].on_io)
    {
      procs_not_on_io_count++;
      procs_not_on_io_info[proc_index++] = procs_info[c];
    }
  }


  //free(procs_info);
  //procs_info = malloc(sizeof(proc_info_t) * procs_not_on_io_count);
  memcpy(procs_info, procs_not_on_io_info, procs_not_on_io_count*sizeof(proc_info_t));
  *procs_count = procs_not_on_io_count;

  //procs_info = procs_not_on_io_info;

  //for(int c=0; c<procs_not_on_io_count; c++)
  //{
  //  procs_info[c] = procs_not_on_io_info[c];
  //}

  //*procs_info = procs_not_on_io_info;
}

//First In First Out
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  io_remover(procs_info, &procs_count);
  if(procs_count == 0)
    return -1;

  //if(procs_info[0].on_io && procs_count>1)
  //  return procs_info[1].pid;
  
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).

  return procs_info[0].pid;
}

//Shortest Job First
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  io_remover(procs_info, &procs_count);
  if(procs_count == 0)
    return -1;

  int min_duration = process_total_time(procs_info[0].pid);
  int selected_proc = procs_info[0].pid;

  for(int c=1; c<procs_count; c++)
  {
    int actual_proc_duration = process_total_time(procs_info[c].pid);
    if(actual_proc_duration < min_duration)
    {
      min_duration = actual_proc_duration;
      selected_proc = procs_info[c].pid;
    }
  }

  
  if(procs_count == 0)
    return -1;
  return selected_proc;
}

//Shortest Time to Completion First
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  io_remover(procs_info, &procs_count);
  if(procs_count == 0)
    return -1;

  int min_time_left = process_total_time(procs_info[0].pid)-procs_info[0].executed_time;
  int selected_proc = procs_info[0].pid;

  for(int c=1; c<procs_count; c++)
  {
    int actual_proc_time_left = process_total_time(procs_info[c].pid)- procs_info[c].executed_time;
    if(actual_proc_time_left < min_time_left)
    {
      min_time_left = actual_proc_time_left;
      selected_proc = procs_info[c].pid;
    }
  }

  
  if(procs_count == 0)
    return -1;
  return selected_proc;
}

int rrindex = 0, slice_time = 1*10; // !!!!!!INVESTIGATE HOW TO ACCESS TIMER INTERRUPT
//Round Robin
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  io_remover(procs_info, &procs_count);
  if(procs_count == 0)
    return -1;

  if(curr_time % slice_time == 0)
  {
    rrindex++;
  }
  
  rrindex %= procs_count;
  
  
  if(procs_count == 0)
    return -1;
  return procs_info[rrindex].pid;
}



// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  else if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;
  else if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;
  else if (strcmp(name, "rr") == 0)
    return *rr_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}


/*
int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;                 // PID del proceso
  int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
                                               // realizando una operación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}
*/