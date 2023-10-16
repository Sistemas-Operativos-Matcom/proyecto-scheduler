#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
int current_pid_index = 0; // Variable utilizada para saber el ultimo proceso que se ejecuto en Round Robin
int slice_time = 0;

int mlfq_pid_index = 0;                // Indice correspondiente al ultimo proceso ejecutado en MLFQ
int mlfq_top_index = 0;                // Cantidad total de procesos activos en MLFQ
int process_queue[MAX_PROCESS_COUNT];  // Array con los pid's de los procesos activos en MLFQ
int priority_queue[MAX_PROCESS_COUNT]; // Array con las prioridades de los procesos activos en MLFQ (existen 3 prioridades)
int slices_queue[MAX_PROCESS_COUNT];   // Array con los slices time de los procesos activos en MLFQ
int queue_counts[3];                   // Array con las cantidades de procesos en cada una de las prioridades en MLFQ
int last_queue = 0;                    // Cola en la que estaba el ultimo proceso ejecutado en MLFQ
int priority_boost = 0;                // Contador para ejecutar el Priority Boost en MLFQ

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

  for (size_t i = 0; i < procs_count; i++) // De todos los procesos, seleccionar el de menor tiempo total de ejecucion
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
  for (size_t i = 0; i < procs_count; i++) // De todos los procesos, seleccionar el de menor tiempo restante de ejecucion
  {
    if (process_total_time(procs_info[i].pid) - procs_info[i].executed_time < min_time)
    {
      min_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      this_pid = procs_info[i].pid;
    }
  }
  return this_pid;
}

int R_R(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) // Aqui el slice time es el propio timer interrupt
{
  if (curr_pid == -1) // Si termino un proceso...
  {
    slice_time = 1;
    if (current_pid_index < procs_count - 1) // Y no es el ultimo, devuelve el proceso con el mismo indice
    {
      return procs_info[current_pid_index].pid;
    }
    current_pid_index = 0; // Si es el ultimo, devuelve el primer proceso
    return procs_info[0].pid;
  }
  if(slice_time < 5)
  { 
    slice_time++;
    return curr_pid;
  }
  slice_time = 1;
  if (current_pid_index < procs_count - 1) // Si el ultimo proceso ejecutado no es el ultimo de la lista, devuelve el siguiente
  {
    current_pid_index++;
    return procs_info[current_pid_index].pid;
  }
  current_pid_index = 0;
  return procs_info[0].pid; // Sino, devuelve el primero
}
int M_L_F_Q(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (curr_pid == -1 && mlfq_top_index != 0) // Si termino un proceso...
  {
    for (size_t i = mlfq_pid_index; i < mlfq_top_index - 1; i++) // Hacer corrimiento a todos los siguientes
    {
      process_queue[i] = process_queue[i + 1];
      priority_queue[i] = priority_queue[i + 1];
      slices_queue[i] = slices_queue[i + 1];
    }
    mlfq_top_index--;           // Decrementar en uno la cantidad de procesos activos
    mlfq_pid_index--;           // Decrementar en uno el indice del ultimo proceso ejecutado
    queue_counts[last_queue]--; // Decrementar en uno la cantidad de procesos en la cola a la que pertenecia el proceso acabado
  }
  if (procs_count > mlfq_top_index) // Si hay nuevos procesos...
  {
    for (size_t i = mlfq_top_index; i < procs_count; i++) // Incorporarlos a todos con la maxima prioridad y aumentar en uno la cantidad de procesos activos
    {
      process_queue[i] = procs_info[i].pid;
      priority_queue[i] = 1;
      queue_counts[0]++;
      slices_queue[i] = 0;
    }
    mlfq_top_index = procs_count;
  }
  priority_boost++;         // Incrementar el contador del priority boost
  if (priority_boost == 50) // Si se alcanza el tope...
  {
    priority_boost = 0;                         // Reiniciar el contador
    for (size_t i = 0; i < mlfq_top_index; i++) // Asignar a todos los procesos la maxima prioridad
    {
      priority_queue[i] = 1;
      slices_queue[i] = 0;
    }
    last_queue = 0;
    queue_counts[0] = mlfq_top_index;
    queue_counts[1] = 0;
    queue_counts[2] = 0;
  }

  if (queue_counts[0] != 0)//Si hay elementos en la maxima prioridad se hace Round Robin sobre ellos, verificando ademas que si se cumple el slice time bajen de prioridad
  {
    int pid = 0;
    for (size_t i = mlfq_pid_index + 1; i < mlfq_top_index; i++)
    {
      if (priority_queue[i] == 1)
      {
        pid = procs_info[i].pid;
        mlfq_pid_index = i;
        slices_queue[i]++;
        if (slices_queue[i] == 5)
        {
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
        pid = procs_info[i].pid;
        mlfq_pid_index = i;
        slices_queue[i]++;
        if (slices_queue[i] == 5)
        {
          last_queue = 1;
          slices_queue[i] = 0;
          priority_queue[i] = 2;
          queue_counts[0]--;
          queue_counts[1]++;
        }
        return pid;
      }
    }
    
  }
  else if (queue_counts[1] != 0)//Si no hay elementos en la maxima prioridad pero si en la segunda se hace Round Robin sobre ellos, verificando ademas que si se cumple el slice time bajen de prioridad
  {
    int pid = 0;
    for (size_t i = mlfq_pid_index + 1; i < mlfq_top_index; i++)
    {
      if (priority_queue[i] == 2)
      {
        pid = procs_info[i].pid;
        mlfq_pid_index = i;
        slices_queue[i]++;
        if (slices_queue[i] == 5)
        {
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
        pid = procs_info[i].pid;
        mlfq_pid_index = i;
        slices_queue[i]++;
        if (slices_queue[i] == 5)
        {
          last_queue = 2;
          slices_queue[i] = 0;
          priority_queue[i] = 3;
          queue_counts[1]--;
          queue_counts[2]++;
        }
        return pid;
      }
    }    
  }
  else    //Por ultimo, si solo hay elementos en la minima prioridad se hace Round Robin sobre ellos, verificando ademas que si se cumple el slice time bajen de prioridad
  {    
    int pid = 0;
    for (size_t i = mlfq_pid_index + 1; i < mlfq_top_index; i++)
    {
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
    for (size_t i = 0; i < mlfq_pid_index + 1; i++)
    {
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
