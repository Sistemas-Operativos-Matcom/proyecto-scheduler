#include "auxiliar.h"

#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

// metodos auxiliares

// Retorna el index de un processo en el array de procs_info o -1 si no se encuentra
int find_pid_array(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return i;
    }
  }
  return -1;
}

// sjf & stcf

// Buscar el menor proceso segun el criterio de comparacion
int select_item(proc_info_t *procs, int count, int comparer(proc_info_t, proc_info_t))
{
  int selected = 0;
  for (int i = 0; i < count; i++)
  {
    if (comparer(procs[i], procs[selected]) < 0)
    {
      selected = i;
    }
  }
  return selected;
}

// MLFQ

// Actualizar los procesos(buscar nuevos y quitar los no activos)
void mlfq_merge(int past_pid[], int level_pid[], int time_pid[], proc_info_t *current_procs, int *past_count, int procs_count)
{
  int ipp = 0;   // index of pid
  int icp = 0;   // index of current pid
  int icopy = 0; // index of copy to past_pid

  // buscar los procesos que aun se mantienen activos
  // como los procesos estan en orden de llegada, a partir de la posicion que este proceso nuevo hacia atras todos
  // seran nuevos en el time interrupt
  while (ipp < (*past_count) && icp < procs_count)
  {
    if (past_pid[ipp] == current_procs[icp].pid) // actualizar los stats del proceso
    {
      past_pid[icopy] = past_pid[ipp];
      level_pid[icopy] = level_pid[ipp];
      time_pid[icopy] = time_pid[ipp];
      icp++;
      icopy++;
    }
    ipp++;
  }

  // annadir  los nuevos
  while (icopy < procs_count)
  {
    past_pid[icopy] = current_procs[icp].pid;
    level_pid[icopy] = 0;
    time_pid[icopy] = 0;
    icopy++;
    icp++;
  }
  *past_count = procs_count; // actualizar el count valido
  return;
}

void mlfq_priority_bost(int levels[], int time_level[], int count)
{
  for (int i = 0; i < count; i++)
  {
    if (levels[i] != 0) // Para no cambiar el time level, de los que ya estaban en el lv 0
    {
      levels[i] = 0;
      time_level[i] = 0;
    }
  }
  return;
}

// Update las propiedades de un proceso y retorna 1 si el proceso consumio todo el tiempo disponible del nivel
int mlfq_update_proc(int pid[], int level[], int time_level[], int MAX_LEVEL, int MAX_TIME_LEVEL, int TIME_SLICE, int index)
{
  time_level[index] += TIME_SLICE;

  if (time_level[index] > MAX_TIME_LEVEL)
  {
    level[index] = (level[index] + 1 >= MAX_LEVEL) ? MAX_LEVEL : level[index] + 1;
    time_level[index] = 0;
    return 1;
  }
  return 0;
}

// buscar el nivel de mas prioridad, temp devuelve la cantidad de items en ese level
int mlfq_find_lowest_depth(int level[], int count, int *temp, int MAX_DEPTH)
{
  int current_depth = MAX_DEPTH;
  *temp = 0;
  for (int i = 0; i < count; i++)
  {
    if (level[i] < current_depth)
    {
      current_depth = level[i];
      *temp = 0;
    }

    if (level[i] == current_depth)
    {
      (*temp)++;
    }
  }
  return current_depth;
}

// Buscar los procesos de la cola actual y guardarlos en dest
void mlfq_filter_procs_level(int level[], proc_info_t *current_procs, int count, int depth, proc_info_t destiny[])
{
  int dest_index = 0;

  for (int i = 0; i < count; i++)
  {
    if (level[i] == depth)
    {
      destiny[dest_index] = current_procs[i];
    }
  }
  return;
}


// QUICK SORT
void swap(void *x, void *y, int size)
{
  char resb[size];
  memcpy(resb, x, size);
  memcpy(x, y, size);
  memcpy(y, resb, size);
}

int get_pivot(int li, int ls) //  Random Pivot Strategy
{
  srand((unsigned)time(NULL));
  return rand() % (ls - li + 1) + li;
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
void q_sort(void *arr, int count, size_t size, int (*compare)(const void *, const void *)) // main
{
  // Quick Sort *InPlace*, Random Pivot, Particion implicita
  // Args: arr:array, count:tamanno del array, size: sizeof(object), compare: criterio de comparacion
  quick_sort(arr, 0, count - 1, size, compare);
  return;
}
