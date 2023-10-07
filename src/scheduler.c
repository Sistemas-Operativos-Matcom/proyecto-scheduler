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
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

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

// Random Scheduler
// Ejecuta un proceso random
int random_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  srand((unsigned)time(NULL));
  return procs_info[rand() % procs_count].pid;
}

// SJF Shortest Job First
int compare_sjf(const void *a, const void *b) // Criterio de Comparacion
{
  proc_info_t *x = (proc_info_t *)a;
  proc_info_t *y = (proc_info_t *)b;

  return (x->on_io && y->on_io) ? 0 : (x->on_io) ? 1
                                  : (y->on_io)   ? -1
                                                 : process_total_time(x->pid) - process_total_time(y->pid);
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int to_exec_index = select(procs_info, procs_count, sizeof(proc_info_t), compare_sjf);
  return procs_info[to_exec_index].pid;
}

// STCF Shortest Time to Completion First Schelduler
int compare_stcf(const void *a, const void *b) // Criterio de Comparacion
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

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int to_exec_index = select(procs_info, procs_count, sizeof(proc_info_t), compare_stcf);
  return procs_info[to_exec_index].pid;
}

// Round Robin
int get_time_slice(int timeInterrup) // parametrizar el time slice
{
  int mult = 2;
  return timeInterrup * mult;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  const int TIME_INTERRUMP = 10; // Time Interrump
  int time_slice = get_time_slice(TIME_INTERRUMP);
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

// Metodos Auxialiares
int find_pid_array(proc_info_t *procs_info, int procs_count, int pid)
{
  // Retorna el index de un processo en el array de procs_info
  // retorna el index del procs, o -1 si no fue encontrado
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return i;
    }
  }
  return -1;
}

int select(void *arr, int count, size_t size, int (*selector)(const void *, const void *))
{
  // Args: array, length, sizeof(item), metodo para seleccionar
  // retorna el index del item
  int selected = 0;
  for (int i = 0; i < count; i++)
  {
    if (selector(arr + selected * size, arr + i * size) < 0) // arr[i] tiene mayor score que arr[selected]
    {
      selected = i;
    }
  }
  return selected;
}

void swap(void *x, void *y, int size) // error haciendo macro para el swap
{
  char resb[size];
  memcpy(resb, x, size);
  memcpy(x, y, size);
  memcpy(y, resb, size);
}

int get_pivot(int li, int ls) // Pivote Random Strategy
{
  srand((unsigned)time(NULL));
  return rand() % (ls - li + 1) + li;
}

void q_sort(void *arr, int count, size_t size, int (*compare)(const void *, const void *)) // main
{
  // Quick Sort *InPlace*, Random Pivot, Particion implicita
  // Args: arr:array, count:tamanno del array, size: sizeof(object), criterio de comparacion
  quick_sort(arr, 0, count - 1, size, compare);
  return;
}

void quick_sort(void *arr, int li, int ls, size_t size, int (*compare)(const void *, const void *))
{
  if (ls <= li || li < 0)
    return;

  swap(arr + ls * size, arr + get_pivot(li, ls) * size, size);
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
  quick_sort(arr, li, index - 1, size, compare);
  quick_sort(arr, index + 1, ls, size, compare);
  return;
}