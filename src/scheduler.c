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
                                               // realizando una opreación IO
  int exec_time = procs_info[0].executed_time; // Tiempo que el proceso se ha
                                               // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}

int candela_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                      int curr_pid)
{
  return procs_info[procs_count - 1].pid;
}

int sjf_sch(proc_info_t *procs_info, int procs_count, int curr_time,
            int curr_pid)
{
  if (curr_pid != -1)
  {
    return curr_pid;
  }
  else
  {
    int tim = __INT_MAX__;
    int thepid = -1;
    for (int i = 0; i < procs_count; i++)
    {
      if (process_total_time(procs_info[i].pid) < tim)
      {
        tim = process_total_time(procs_info[i].pid);
        thepid = procs_info[i].pid;
      }
    }
    return thepid;
  }
}
int stcf_sch(proc_info_t *procs_info, int procs_count, int curr_time,
             int curr_pid)
{

  int tim = __INT_MAX__;
  int thpid = -1;
  for (int i = 0; i < procs_count; i++)
  {
    int rem = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    if (rem < tim)
    {
      tim = rem;
      thpid = procs_info[i].pid;
    }
  }
  return thpid;
}

int curr = 0;
int slicerr = 50;
int rounr_sch(proc_info_t *procs_info, int procs_count, int curr_time,
              int curr_pid)
{
  if (curr_pid == -1)
  {
    return procs_info[0].pid;
  }
  if (curr_time % slicerr == 0)
  {
    curr = (curr + 1) % procs_count;
  }
  curr = (curr) % procs_count;
  return procs_info[curr].pid;
}

int roundrobin_sch(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  if (curr_pid == -1)
  {
    return procs_info[0].pid;
  }
  if (curr_time % slicerr == 0)
  {
    curr = (curr + 1) % procs_count;
  }
  curr = (curr) % procs_count;
  if (procs_info[curr].on_io)
  {
    int res = -1;
    for (int i = 0; i < procs_count; i++)
    {
      if (!procs_info[i].on_io)
      {
        res = procs_info[i].pid;
        break;
      }
    }
    if (res != -1)
    {
      return res;
    }
  }
  return procs_info[curr].pid;
}

#define cantidad_de_prioridades 10
#define slice_time 30
int boost = slice_time * 69;
int curr_execumulation = 0;
int prevpidind = 0;

int mqmf_sch(proc_info_t *procs_info, int procs_count, int curr_time,
             int curr_pid)
{
  prevpidind = prevpidind % procs_count;

  for (int i = 0; i < procs_count; i++)
  {
    // actualizo la prioridad de los procesos nuevos a 0, la mas alta
    if (procs_info[i].executed_time <= 0)
    {
      procs_info[i].priority = 0;
      procs_info[i].acumulativetime = 0;
    }
  }

  if (curr_time % boost == 0)
  {
    for (int i = 0; i < procs_count; i++)
    {
      procs_info[i].priority = 0;
      procs_info[i].acumulativetime = 0;
    }
  }
  if (curr_pid != -1)
  {
    for (int i = 0; i < procs_count; i++)
    {
      if (curr_pid == procs_info[i].pid)
      {
        procs_info[i].acumulativetime += (procs_info[i].executed_time - curr_execumulation);
      }
    }
  }

  if (curr_time % slice_time == 0)
  {
    for (int i = 0; i < procs_count; i++)
    {
      if (procs_info[i].acumulativetime >= slice_time && procs_info[i].priority < cantidad_de_prioridades - 1)
      {
        procs_info[i].priority += 1;
        procs_info[i].acumulativetime = 0;
      }
    }
  }

  /* Comprobando prioridades q cambian y tienen el boost
  for (int i = 0; i < procs_count; i++)
  {
    printf("%d ", procs_info[i].priority);
  }
  printf("\n");
  //*/

  int retpid = -1;
  int actprior = cantidad_de_prioridades;

  // Esto busca hacia delante el primer proceso q no este en io con la mayor(menor, las puse al reves)
  for (int i = prevpidind + 1; i < prevpidind + procs_count + 1; i++)
  {
    //printf("(%d, %d, %d)", procs_info[i % procs_count].priority,actprior, i % procs_count);
    if (procs_info[i % procs_count].priority < actprior && !(procs_info[i % procs_count].on_io))
    {
      //printf("[selected] ");
      actprior = procs_info[i % procs_count].priority;
      retpid = procs_info[i % procs_count].pid;
      curr_execumulation = procs_info[i % procs_count].executed_time;
      prevpidind = i % procs_count;
    }
  }
  printf("\n");
  return retpid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name)
{
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0)
    return *fifo_scheduler;
  if (strcmp(name, "candela") == 0)
    return *candela_scheduler;
  if (strcmp(name, "sjf") == 0)
    return *sjf_sch;
  if (strcmp(name, "stcf") == 0)
    return *stcf_sch;
  if (strcmp(name, "rri") == 0)
    return *rounr_sch;
  if (strcmp(name, "rr") == 0)
    return *roundrobin_sch;
  if (strcmp(name, "mlfq") == 0)
    return *mqmf_sch;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
