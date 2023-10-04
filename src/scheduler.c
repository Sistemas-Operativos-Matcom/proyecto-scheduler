#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"
#include "simulation.h"
#define QUANTUM 50

// Queue ready_queue;

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
  if (curr_pid == -1)
  {
    printf("\nCAMBIA\n");
    /* code */
  }
  else
  {
    // int exec_time = procs_info[0].executed_time;
    // printf("\n%i\n", process_total_time(procs_info[0].pid));
  }

  return procs_info[0].pid;
}
int my_SJF(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid == -1)
  {
    proc_info_t procmin = procs_info[0];
    int TTmin = process_total_time(procmin.pid);
    for (int i = 1; i < procs_count; i++)
    {
      int temp = process_total_time(procs_info[i].pid);
      if (temp < TTmin)
      {
        procmin = procs_info[i];
        TTmin = temp;
      }
    }
    return procmin.pid;
  }
  else
  {
    return curr_pid;
  }
}
int my_STCF(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  proc_info_t procmin = procs_info[0];
  int TTmin = process_total_time(procmin.pid);
  for (int i = 1; i < procs_count; i++)
  {
    int temp = process_total_time(procs_info[i].pid);
    if (temp < TTmin)
    {
      procmin = procs_info[i];
      TTmin = temp;
    }
  }
  return procmin.pid;
}
int my_RR(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  static int pos = -1;
  if (curr_time % QUANTUM == 0 || curr_pid == -1)
    return procs_info[++pos % procs_count].pid;
  return curr_pid;
}

/* int findPID(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return i;
    }
  }
  return -1;
} */
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

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // initQueue(&ready_queue);

  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0)
    return *my_SJF;
  if (strcmp(name, "stcf") == 0)
    return *my_STCF;
  if (strcmp(name, "rr") == 0)
    return *my_RR;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
