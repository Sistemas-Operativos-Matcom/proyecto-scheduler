#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "queue.h"

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
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  if( curr_pid != -1 ) return curr_pid;

  int process_duration_i;
  int process_duration_j;

  for ( int i = 0 ; i < procs_count ; i++ ) 
  {
    process_duration_i = process_total_time(procs_info[i].pid);

    for( int j = i+1 ; j < procs_count ; j++ )
    {
      process_duration_j = process_total_time(procs_info[j].pid);

      if (process_duration_i > process_duration_j) 
      {
        proc_info_t temp = procs_info[i];
        procs_info[i] = procs_info[j];
        procs_info[j] = temp;
        process_duration_i = process_duration_j;
      }
    }
  }

  return procs_info[0].pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{

  int process_duration_i;
  int process_duration_j;

  for ( int i = 0 ; i < procs_count ; i++ ) 
  {
    process_duration_i = process_total_time(procs_info[i].pid)-procs_info[i].executed_time;

    for( int j = i+1 ; j < procs_count ; j++ )
    {
      process_duration_j = process_total_time(procs_info[j].pid)-procs_info[j].executed_time;

      if (process_duration_i > process_duration_j) 
      {
        proc_info_t temp = procs_info[i];
        procs_info[i] = procs_info[j];
        procs_info[j] = temp;
        process_duration_i = process_duration_j;
      }
    }
  }

  return procs_info[0].pid;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  static int slice_time = 1;
  static int index = 0 ;

  if(index >= procs_count) index = 0;

  if(slice_time <= 3) 
  {
    slice_time++;
    return procs_info[index].pid;
  }
  else
  {
    if(index + 1 < procs_count) index++;
    else index=0;
    slice_time = 1;
    return procs_info[index].pid;
  }
}

static struct Queue queues[3];
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) 
{
  static int que_turn = 0;
  static int last_pid = -1;

  for(int i = 0 ; i < procs_count ; i++ )
  {
    if ( (procs_info[i].executed_time == 0) && !find_process(&queues[0],procs_info[i].pid) )
    {
      enqueue(procs_info[i].pid,&queues[0]);
    }
  }

  if ( curr_time % 90 == 0 )
  {
    if ( !isEmpty( &queues[ (que_turn + 1) % 3 ] ) )
    {
      que_turn = (que_turn + 1) % 3;
    }
    else if ( !isEmpty( &queues[ (que_turn + 2) % 3 ] ) )
    {
      que_turn = (que_turn + 2) % 3;
    }
  }

  if( curr_time % 600 == 0 )
  {
    while( queues[1].size != 0 )
    {
      int current = dequeue(&queues[1]);
      enqueue(current,&queues[0]);
    }
    while( queues[2].size != 0 )
    {
      int current = dequeue(&queues[2]);
      enqueue(current,&queues[0]);
    }

    que_turn = 0; 
  }

  if ( isEmpty(&queues[que_turn]) )
  {
    if ( isEmpty(&queues[(que_turn + 1) % 3]) )
    {
      if ( isEmpty(&queues[(que_turn +2) % 3]) )
      {
        last_pid = -1;
        return -1;
      }
      else
      {
        que_turn = (que_turn +2) % 3;
      }
    }
    else
    {
      que_turn = (que_turn +1) % 3;
    }
  }

  int valid_pid = 0;
  int current = dequeue(&queues[que_turn]);

  for(int j = 0; j < 500;j++)
  {
    for(int i = 0;i<procs_count;i++)
    {
      if(current==procs_info[i].pid)
      {
        valid_pid=1;
        break;
      }
    }
    if(valid_pid)
    {
      break;
    }
    current = dequeue(&queues[que_turn]);
    if(current == -1)
    {
      que_turn= (que_turn + 1 ) % 3;
    }
  }

  enqueue(current,&queues[(que_turn + 1) % 3]);

  last_pid = current;
  return current;
}

schedule_action_t get_scheduler(const char *name) 
{
  for (int i = 0; i < 3; i++) 
  {
    initializeQueue(&queues[i],10000);
  }

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  else if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  else if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  else if (strcmp(name, "rr") == 0) return *round_robin_scheduler;
  else if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
