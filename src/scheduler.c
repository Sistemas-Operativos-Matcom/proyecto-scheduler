#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
int current_pid_index = 0;
int mlfq_pid_index = 0;
int mlfq_top_index = 0;
int process_queue[MAX_PROCESS_COUNT];
int priority_queue[MAX_PROCESS_COUNT];
int queue_counts[3];
int current_queue = 0;
int last_queue = 0;
int slices_queue[MAX_PROCESS_COUNT];
int priority_boost = 0;

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
int S_J_F(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int min_time = __INT_MAX__;
  int this_pid = 0;
  if (curr_pid != -1)
  {
    return curr_pid;
  }

  for (size_t i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) < min_time)
    {
      min_time = process_total_time(procs_info[i].pid);
      this_pid = procs_info[i].pid;
    }
  }
  return this_pid;
}
int S_T_F_C(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int min_time = __INT_MAX__;
  int this_pid = 0;
  for (size_t i = 0; i < procs_count; i++)
  {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < min_time)
    {
      min_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      this_pid = procs_info[i].pid;
    }
  }
  return this_pid;
}

int R_R(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid == -1)
  {
    if (current_pid_index < procs_count - 1)
    {
      current_pid_index;
      return procs_info[current_pid_index].pid;
    }
    current_pid_index = 0;
    return procs_info[0].pid;
  }

  for (size_t i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == curr_pid)
    {
      if (i < procs_count - 1)
      {
        current_pid_index = i + 1;
        return procs_info[i + 1].pid;
      }
      current_pid_index = 0;
      return procs_info[0].pid;
    }
  }
}
int M_L_F_Q(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // printf("Hola\n");
  if (curr_pid == -1 && mlfq_top_index != 0)
  {
    // printf("Termino el proceso:%d\n",mlfq_pid_index);
    for (size_t i = mlfq_pid_index; i < mlfq_top_index - 1; i++)
    {
      // printf("Corri al proceso %d con prioridad%d\n",process_queue[i+1],priority_queue[i+1]);
      process_queue[i] = process_queue[i + 1];
      priority_queue[i] = priority_queue[i + 1];
      slices_queue[i] = slices_queue[i + 1];
    }
    mlfq_top_index--;
    mlfq_pid_index--;
    queue_counts[last_queue]--;
  }
  if (procs_count > mlfq_top_index)
  {
    // printf("Hay gente nueva\n");
    current_queue = 0;
    for (size_t i = mlfq_top_index; i < procs_count; i++)
    {
      // printf("Llego el proceso %d\n",procs_info[i].pid);
      process_queue[i] = procs_info[i].pid;
      priority_queue[i] = 1;
      queue_counts[0]++;
      slices_queue[i] = 0;
    }
    mlfq_top_index = procs_count;
  }
  priority_boost++;
  if (priority_boost == 50)
  {
    // printf("Priority Boost\n");
    priority_boost = 0;
    for (size_t i = 0; i < mlfq_top_index; i++)
    {
      priority_queue[i] = 1;
      slices_queue[i] = 0;
    }
    current_queue = 0;
    last_queue = 0;
    queue_counts[0] = mlfq_top_index;
    queue_counts[1] = 0;
    queue_counts[2] = 0;
  }

  if (current_queue == 0)
  {
    if (queue_counts[0] == 0)
    {
      current_queue = 1;
    }
    else
    {
      // printf("Priority:1\n");
      int pid = 0;
      int found = 0;
      for (size_t i = mlfq_pid_index + 1; i < mlfq_top_index; i++)
      {
        if (priority_queue[i] == 1)
        {
          found++;
          pid = procs_info[i].pid;
          mlfq_pid_index = i;
          slices_queue[i]++;
          if (slices_queue[i] == 5)
          {
            // printf("El proceso %d baja a prioridad %d\n",mlfq_pid_index,2);
            last_queue = 1;
            slices_queue[i] = 0;
            priority_queue[i] = 2;
            queue_counts[0]--;
            queue_counts[1]++;
          }
          return pid;
        }
      }
      for (size_t i = 0; i < mlfq_pid_index + 1; i++)
      {
        if (priority_queue[i] == 1)
        {
          found++;
          pid = procs_info[i].pid;
          mlfq_pid_index = i;
          slices_queue[i]++;
          if (slices_queue[i] == 5)
          {
            // printf("El proceso %d baja a prioridad %d\n",mlfq_pid_index,2);
            last_queue = 1;
            slices_queue[i] = 0;
            priority_queue[i] = 2;
            queue_counts[0]--;
            queue_counts[1]++;
          }
          return pid;
        }
      }
      if (found == 0)
      {
        printf("Mistake\n");
        printf("Cola actual:%d\n", current_queue);
        printf("Cola actual:%d\n", current_queue);
        printf("Prioridad 1:%d\n", queue_counts[0]);
        printf("Prioridad 2:%d\n", queue_counts[1]);
        printf("Prioridad 3:%d\n", queue_counts[2]);
        printf("Total:%d\n", mlfq_top_index);
        printf("Total Real:%d\n", procs_count);
        printf("PID index:%d\n", mlfq_pid_index);
        for (size_t i = 0; i < mlfq_top_index; i++)
        {
          printf("%d\n", priority_queue[i]);
        }
      }
    }
  }
  if (current_queue == 1)
  {
    if (queue_counts[1] == 0)
    {
      current_queue = 2;
    }
    else
    {
      // printf("Priority:2\n");
      int pid = 0;
      int found = 0;
      for (size_t i = mlfq_pid_index + 1; i < mlfq_top_index; i++)
      {
        if (priority_queue[i] == 2)
        {
          found++;
          pid = procs_info[i].pid;
          mlfq_pid_index = i;
          slices_queue[i]++;
          if (slices_queue[i] == 5)
          {
            // printf("El proceso %d baja a prioridad %d\n",mlfq_pid_index,3);
            last_queue = 2;
            slices_queue[i] = 0;
            priority_queue[i] = 3;
            queue_counts[1]--;
            queue_counts[2]++;
          }
          return pid;
        }
      }
      for (size_t i = 0; i < mlfq_pid_index + 1; i++)
      {
        if (priority_queue[i] == 2)
        {
          found++;
          pid = procs_info[i].pid;
          mlfq_pid_index = i;
          slices_queue[i]++;
          if (slices_queue[i] == 5)
          {
            // printf("El proceso %d baja a prioridad %d\n",mlfq_pid_index,3);
            last_queue = 2;
            slices_queue[i] = 0;
            priority_queue[i] = 3;
            queue_counts[1]--;
            queue_counts[2]++;
          }
          return pid;
        }
      }
      if (found == 0)
      {
        printf("Mistake\n");
        printf("Cola actual:%d\n", current_queue);
        printf("Prioridad 1:%d\n", queue_counts[0]);
        printf("Prioridad 2:%d\n", queue_counts[1]);
        printf("Prioridad 3:%d\n", queue_counts[2]);
        printf("Total:%d\n", mlfq_top_index);
        printf("Total Real:%d\n", procs_count);
        printf("PID index:%d\n", mlfq_pid_index);
        for (size_t i = 0; i < mlfq_top_index; i++)
        {
          printf("%d\n", priority_queue[i]);
        }
      }
    }
  }
  if (current_queue == 2)
  {
    // printf("Priority:3\n");
    int pid = 0;
    int found = 0;
    for (size_t i = mlfq_pid_index + 1; i < mlfq_top_index; i++)
    {
      if (priority_queue[i] == 3)
      {
        found++;
        pid = procs_info[i].pid;
        mlfq_pid_index = i;
        slices_queue[i]++;
        if (slices_queue[i] == 5)
        {
          slices_queue[i] = 0;
        }
        return pid;
      }
    }
    for (size_t i = 0; i < mlfq_pid_index + 1; i++)
    {
      found++;
      if (priority_queue[i] == 3)
      {
        pid = procs_info[i].pid;
        mlfq_pid_index = i;
        slices_queue[i]++;
        if (slices_queue[i] == 5)
        {
          slices_queue[i] = 0;
        }
        return pid;
      }
    }
    if (found == 0)
    {
      printf("Mistake\n");
      printf("Cola actual:%d\n", current_queue);
      printf("Prioridad 1:%d\n", queue_counts[0]);
      printf("Prioridad 2:%d\n", queue_counts[1]);
      printf("Prioridad 3:%d\n", queue_counts[2]);
      printf("Total:%d\n", mlfq_top_index);
      printf("Total Real:%d\n", procs_count);
      printf("PID index:%d\n", mlfq_pid_index);
      for (size_t i = 0; i < mlfq_top_index; i++)
      {
        printf("%d\n", priority_queue[i]);
      }
    }
  }
  return -1;
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

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0)
    return *S_J_F;
  if (strcmp(name, "sctf") == 0)
    return *S_T_F_C;
  if (strcmp(name, "rr") == 0)
    return *R_R;
  if (strcmp(name, "mlfq") == 0)
    return *M_L_F_Q;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
