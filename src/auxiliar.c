#include "auxiliar.h"

#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

// Metodos Auxialiares

// Los metodos estan generales por flexibilidad del proyecto
int find_pid_array(proc_info_t *procs_info, int procs_count, int pid)
{
  // Retorna el index de un processo en el array de procs_info o -1 si no se encuentra
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      return i;
    }
  }
  return -1;
}

int select_item(void *arr, int count, size_t size, int (*compare)(const void *, const void *))
{
  // Siempre se queda con el menor segun compare
  // Args: array, length, sizeof(item), metodo para seleccionar
  // retorna el index del item
  int selected = 0;
  for (int i = 0; i < count; i++)
  {
    if (compare(arr + selected * size, arr + i * size) > 0) // arr[i] tiene mayor score que arr[selected]
    {
      selected = i;
    }
  }
  return selected;
}

// quick sort
void swap(void *x, void *y, int size) // error haciendo macro para el swap
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
