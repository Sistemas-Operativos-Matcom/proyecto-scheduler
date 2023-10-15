#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int last_proc_pos = 0;

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
                   int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;                 // PID del proceso
  int on_io = procs_info[0].on_io;             // Indica si el proceso se encuentra
                                               // realizando una opreación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}

int SJF(proc_info_t *procs_info, int procs_count, int curr_time,
        int curr_pid)
{
  if (curr_pid != -1)
    return curr_pid;

  if (procs_count == 0)
    return -1;

  int min_pid = procs_info[0].pid;
  for (int i = 1; i < procs_count; i++)
  {
    int i_pid = procs_info[i].pid;
    if (process_total_time(i_pid) < process_total_time(min_pid))
    {
      min_pid = i_pid;
    }
  }
  return min_pid;
}

int STCF(proc_info_t *procs_info, int procs_count, int curr_time,
         int curr_pid)
{
  if (procs_count == 0)
    return -1;

  proc_info_t minProc = procs_info[0];

  for (int i = 1; i < procs_count; i++)
  {
    proc_info_t iProc = procs_info[i];
    int i_time_to_completion = process_total_time(iProc.pid) - iProc.executed_time;
    int min_time_to_completion = process_total_time(minProc.pid) - minProc.executed_time;
    if (i_time_to_completion < min_time_to_completion)
    {
      minProc = iProc;
      min_time_to_completion = i_time_to_completion;
    }
  }
  return minProc.pid;
}

int Round_Robin(proc_info_t *procs_info, int procs_count, int curr_time,
                int curr_pid)
{
// printf("         ");
//    printf("%d",curr_time);
//   printf("  ");
//    printf("%d",curr_pid); 
//   printf("  ");
//   printf("%d",last_proc_pos); printf("  ");
//    printf("%d",procs_count);
//    printf("\n");

  if (procs_count == 0)
    return -1;

  if (curr_pid == -1)
  {
    if (last_proc_pos >= procs_count)
      last_proc_pos = 0;
    return procs_info[last_proc_pos].pid;
  }

  if (last_proc_pos + 1 >= procs_count)
  {
    last_proc_pos = 0;
  }
  else
    last_proc_pos += 1;

 
  return procs_info[last_proc_pos].pid;

}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0)
    return *SJF;
  if (strcmp(name, "stcf") == 0)
    return *STCF;
  if (strcmp(name, "rr") == 0)
    return *Round_Robin;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
