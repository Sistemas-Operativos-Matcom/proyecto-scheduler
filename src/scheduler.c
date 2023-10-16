#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

#define SLICE_TIME 30
#define PRIORITY_BOOST_TIME 300
#define LEVELS 3

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

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if(procs_count == 0) return -1;

  int current_pid = procs_info[0].pid;
  int current_time =  process_total_time(current_pid);

  for(int i=1; i<procs_count; i++)
  {
    int pid = procs_info[i].pid;
    int exec_time = process_total_time(pid);

    if(exec_time < current_time)
    {
      current_pid = pid;
      current_time = exec_time;
    }
  }

  return current_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if(procs_count == 0) return -1;

  int current_pid = procs_info[0].pid;
  int current_time =  process_total_time(current_pid) - procs_info[0].executed_time;

  for(int i=1; i<procs_count; i++)
  {
    int pid = procs_info[i].pid;
    int exec_time = process_total_time(pid) - procs_info[i].executed_time;

    if(exec_time < current_time)
    {
      current_pid = pid;
      current_time = exec_time;
    }
  }

  return current_pid;
}

int ind = 0;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if(procs_count == 0) return -1;

  ind = ind % procs_count; 

  if(curr_time % SLICE_TIME == 0 && procs_info[ind].pid == curr_pid) 
    ind = (1 + ind) % procs_count;

  return procs_info[ind].pid;

}

int io_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if(procs_count == 0) return -1;

  ind = ind % procs_count;

  int on_io = 0;

  if(curr_time % SLICE_TIME == 0 && procs_info[ind].pid == curr_pid)
    ind = (1 + ind) % procs_count;

  while(procs_info[ind].on_io != 0)
  {
    on_io += 1;
    
    ind = (ind + 1) % procs_count;

    if(on_io == procs_count) return -1;
  }

  return procs_info[ind].pid;
}

// mlfq arrays
int mlfq_pid[5000];
int mlfq_level[5000];
int mlfq_slice_time[5000];
int mlfq_arrive_time[5000];

// hacer coincidir las posiciones de mis mlfq arrays con las de los procs info
void update_mlfq_array(proc_info_t *procs_info, int procs_count, int curr_time)
{

  for(int i=0, j=0; i<procs_count; i++, j++)
  {
    if(procs_info[i].pid == mlfq_pid[j])
    {
      mlfq_pid[i] = mlfq_pid[j];
      mlfq_level[i] = mlfq_arrive_time[j];
      mlfq_slice_time[i] = mlfq_slice_time[j];
      mlfq_arrive_time[i] = mlfq_arrive_time[j]; 
    }
    else if(mlfq_pid[j] == -1)
    {
      mlfq_pid[i] = procs_info[i].pid;
      mlfq_level[i] = 0;
      mlfq_slice_time[i] = SLICE_TIME;
      mlfq_arrive_time[i] = curr_time;
    }
    else
      i--;
  }
}

void priority_boost(int procs_count)
{
  for(int i=0; i<procs_count; i++)
  {
    mlfq_level[i] = 0;
  }
}

int find_index(proc_info_t *procs_info, int procs_count, int curr_pid)
{
  for(int i=0; i<procs_count; i++)
  {
    if(procs_info[i].pid == curr_pid) return i;
  }
  return -1;
}

int fp(proc_info_t *procs_info, int procs_count)
{
  for(int i=0; i<procs_count; i++)
  {
    if(!procs_info[i].on_io)
    {
      return i;
    }
  }
  return -1;
}

int lowest_level(proc_info_t *procs_info, int procs_count)
{
  int j = fp(procs_info, procs_count);
  for(int i=0; i<procs_count; i++)
  {
    if(!procs_info[i].on_io)
    {
      j = (mlfq_level[i] < mlfq_level[j]) ? i : j;
    }
  }
  return j;
}

int different_level(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid, int lw)
{
  int j = lw;
  for(int i=j+1; i<procs_count; i++)
  {
    if(mlfq_level[i] == mlfq_level[j] && mlfq_arrive_time[i] < mlfq_arrive_time[j] && !procs_info[i].on_io)
    {
      j = i;
    }
  }
  return j;
}

int same_level(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid, int lw, int last_arrive)
{
  int j = lw;
  for(int i = lw+1; i<procs_count; i++)
  {
    if(mlfq_level[i] == mlfq_level[j] && !procs_info[i].on_io)
    {
      int i_arrive_time = mlfq_arrive_time[i] + (mlfq_arrive_time[i] < last_arrive) ? curr_time : 0; 
      int j_arrive_time = mlfq_arrive_time[j] + (mlfq_arrive_time[j] < last_arrive) ? curr_time : 0; 

      if(i_arrive_time < j_arrive_time) j = i;
    }
  }

  return j;
}

int last_index = -1;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if(procs_count == 0) return -1;

  int last_arrive = 0;
  int last_level = LEVELS + 1;
  
  if(last_index != -1)
  {
    last_arrive = mlfq_arrive_time[last_index];
    last_level = mlfq_level[last_index];
  }
  
  update_mlfq_array(procs_info, procs_count, curr_time);

  int index = find_index(procs_info, procs_count, curr_pid);

  int lw = lowest_level(procs_info, procs_count);

  if(lw == -1) return -1;

  if(mlfq_slice_time[index] <= 0)
  {
    mlfq_level[index] = (mlfq_level[index] >= LEVELS) ? LEVELS : mlfq_level[index] + 1;  
    mlfq_arrive_time[index] = curr_time;
    mlfq_slice_time[index] = SLICE_TIME;
  }

  if(curr_time % PRIORITY_BOOST_TIME == 0) priority_boost(procs_count);

  int k = 0;
  if(last_level != mlfq_level[lw])
  {
    k = different_level(procs_info, procs_count, curr_time, curr_pid, lw);
  }
  else
  {
    k = same_level(procs_info, procs_count, curr_time, curr_pid, lw, last_arrive);
  }

  mlfq_slice_time[k] -= 10;
  last_index = k;

  return procs_info[k].pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  memset(mlfq_pid, -1, 5000*sizeof(int));

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  if (strcmp(name, "io") == 0) return *io_scheduler;

  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
