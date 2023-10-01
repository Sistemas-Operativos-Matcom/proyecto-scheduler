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
                                               // realizando una operación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}

// Random Scheduler
int random_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  srand((unsigned)time(NULL));
  return procs_info[rand() % procs_count].pid;
}

// Quick Sort Generic Func ///////////////////////////////////////////////
void swap(void *x, void *y, int size) // problems with macro SWAP?
{
  char resb[size];
  memcpy(resb, x, size);
  memcpy(x, y, size);
  memcpy(y, resb, size);
}

int compare(const void *a, const void *b) // Example of compare func
{
  return *(int *)a - *(int *)b;
}

int getPivot(int li, int ls) // Strategy: Random Pivot (Implicit Partition)
{
  srand((unsigned)time(NULL));
  return rand() % (ls - li + 1) + li;
}

void qSort(void *arr, int count, size_t size, int (*compare)(const void *, const void *)) // In_place
{
  quickSort(arr, 0, count - 1, size, compare);
  return;
}

void quickSort(void *arr, int li, int ls, size_t size, int (*compare)(const void *, const void *)) // Private Func, Recursive Calls
{
  if (ls <= li || li < 0)
  {
    return;
  }
  swap(arr + ls * size, arr + getPivot(li, ls) * size, size); // pv -> last pos
  int index = li;
  for (int i = li; i < ls; i++)
  {
    if (compare(arr + i * size, arr + ls * size) < 0)
    {
      swap(arr + index * size, arr + i * size, size);
      index++;
    }
  }
  swap(arr + index * size, arr + ls * size, size);
  quickSort(arr, li, index - 1, size, compare);
  quickSort(arr, index + 1, ls, size, compare);
  return;
}
// End QSort /////////////////////////////////////////////////////////////

// SJF Shortest Job First
int compareSJF(const void *a, const void *b) // Example of compare func
{
  proc_info_t *x = (proc_info_t *)a;
  proc_info_t *y = (proc_info_t *)b;

  return (x->on_io && y->on_io) ? 0 : (x->on_io) ? 1
                                  : (y->on_io)   ? -1
                                                 : process_total_time(x->pid) - process_total_time(y->pid);
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid)
{
  int currentProc = 0;

  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == curr_pid)
    {
      currentProc = i;
    }
  }

  if (curr_time != 0 && !procs_info[currentProc].on_io)
  {
    return curr_pid;
  }

  qSort(procs_info, procs_count, sizeof(proc_info_t), compareSJF);
  return procs_info[0].pid;
}

// STCF Shortest Time to Completion First Schelduler
int compareSTCF(const void *a, const void *b) // Example of compare func
{
  proc_info_t *x = (proc_info_t *)a;
  proc_info_t *y = (proc_info_t *)b;

  if (x->on_io && y->on_io)
    return 0;

  if (x->on_io)
    return -1;

  if (y->on_io)
    return 1;

  int xTComplete = process_total_time(x->pid) - x->executed_time;
  int yTComplete = process_total_time(y->pid) - y->executed_time;
  return xTComplete - yTComplete;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  qSort(procs_info, procs_count, sizeof(proc_info_t), compareSTCF);
  return procs_info[0].pid;
}

// Round Robin

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
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
// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //
  if (strcmp(name, "random") == 0)
    return *random_scheduler;

  if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
