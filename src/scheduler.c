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

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una operación IO
  int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}


int random_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  srand((unsigned)time(NULL));
  return procs_info[rand()%procs_count].pid;
}


// Quick Sort Generic Func 
void swap(void *x, void *y, int size) // problems with macro SWAP
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

int getPivot(int li, int ls) // Strategy Random Pivot
{
    srand((unsigned)time(NULL));
    return rand() % (ls - li + 1) + li;
}

void qSort(void *arr, int count, size_t size, int (*compare)(const void *, const void *)) // In-place
{
    quickSort(arr, 0, count - 1, size, compare);
    return;
}

void quickSort(void *arr, int li, int ls, size_t size, int (*compare)(const void *, const void *)) //Private Func, Recursive Calls
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
// End QSort

// STCF Shortest Time to Completion First) Schelduler
int STCF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //CPU

  

  //IO

  return -1;
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
