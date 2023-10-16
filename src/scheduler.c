#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

// La función que define un scheduler está compuesta por los siguientes
// parámetros:
//
//  - procs_info: Array queroo contiene la información de cada proceso activo
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

typedef struct proc_queue
{
  proc_info_t q[MAX_PROCESS_COUNT];
  int sz;

} proc_queue_t;

const int time_slice = 3 * 10;
proc_queue_t *q1, *q2, *q3;
char inited = 0;

void init()
{
  if (inited == 1)
    return;
  inited = 1;

  q1 = (proc_queue_t *)malloc(sizeof(proc_queue_t));
  q2 = (proc_queue_t *)malloc(sizeof(proc_queue_t));
  q3 = (proc_queue_t *)malloc(sizeof(proc_queue_t));
}

void remove_pid(proc_info_t proc, proc_queue_t *queue)
{
  proc_info_t aux[MAX_PROCESS_COUNT];
  int curr = 0;

  for (int i = 0; i < queue->sz; i++)
    if (queue->q[i].pid != proc.pid)
      aux[curr++] = queue->q[i];

  for (int idx = 0; idx < curr; idx++)
    queue->q[idx] = aux[idx];

  queue->sz = curr;
}

void add_pid(proc_info_t proc, proc_queue_t *queue)
{

  queue->q[queue->sz] = proc;
  queue->sz++;
}

void update_info(proc_info_t *procs_info, int procs_count)
{
  for (int pc = 0; pc < procs_count; pc++)
  {
    proc_info_t curr_proc = procs_info[pc];
    for (int idx = 0; idx < q1->sz; idx++)
      if (q1->q[idx].pid == curr_proc.pid)
        q1->q[idx] = curr_proc;

    for (int idx = 0; idx < q2->sz; idx++)
      if (q2->q[idx].pid == curr_proc.pid)
        q2->q[idx] = curr_proc;

    for (int idx = 0; idx < q3->sz; idx++)
      if (q3->q[idx].pid == curr_proc.pid)
        q3->q[idx] = curr_proc;
  }
}
int is_on_queue(int pid)
{
  for (int idx = 0; idx < q1->sz; idx++)
    if (q1->q[idx].pid == pid)
      return 1;

  for (int idx = 0; idx < q2->sz; idx++)
    if (q2->q[idx].pid == pid)
      return 1;

  for (int idx = 0; idx < q3->sz; idx++)
    if (q3->q[idx].pid == pid)
      return 1;

  return 0;
}
proc_info_t get_proc_by_pid(int pid, proc_info_t *procsinf, int proc_count)
{
  for (int idx = 0; idx < proc_count; idx++)
    if (procsinf[idx].pid == pid)
      return procsinf[idx];
}
int is_active(proc_info_t proc, proc_info_t *procs_info, int proc_count)
{
  for (int idx = 0; idx < proc_count; idx++)
    if (procs_info[idx].pid == proc.pid)
      return 1;
  return 0;
}
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                 int curr_pid)
{
  if (curr_pid == -1)
    return procs_info[0].pid;

  if (procs_count == 1)
    return procs_info[0].pid;

  int active_pr_count = 0;

  for (int idx = 0; idx < procs_count; idx++)
  {
    if (procs_info[idx].executed_time <= process_total_time(procs_info[idx].pid))
      active_pr_count += 1;
  }

  proc_info_t *active_process = (proc_info_t *)malloc(active_pr_count * sizeof(proc_info_t));
  int curr = 0;

  for (int idx = 0; idx < procs_count; idx++)
    if (procs_info[idx].executed_time <= process_total_time(procs_info[idx].pid))
      active_process[curr++] = procs_info[idx];

  for (int idx = 0; idx < active_pr_count; idx++)
    if (active_process[idx].pid == curr_pid && active_process[idx].executed_time % time_slice == 0)
      return active_process[(idx + 1) % procs_count].pid;

  // Return same pid if process is still in  it's time slice execution time
  return curr_pid;
}
int S = 100;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  init();
  if (procs_count == 0)
    return curr_pid;
  for (int idx = 0; idx < procs_count; idx++)
    if (!is_on_queue(procs_info[idx].pid) && is_active(procs_info[idx], procs_info, procs_count))
      add_pid(procs_info[idx], q1);

  update_info(procs_info, procs_count);

  proc_info_t curr_proc = get_proc_by_pid(curr_pid, procs_info, procs_count);

  // Change Priority

  int flag = 0;

  for (int idx = 0; idx < q1->sz; idx++)
  {
    if (q1->q[idx].executed_time % time_slice == 0 && q1->q[idx].executed_time)
    {
      flag = 1;
      add_pid(q1->q[idx], q2);
      remove_pid(q1->q[idx], q1);
    }
  }
  // Change Priority
  if (flag == 0)
  {
    for (int idx = 0; idx < q2->sz; idx++)
    {
      if (q2->q[idx].executed_time % time_slice == 0 && q2->q[idx].executed_time)
      {
        add_pid(q2->q[idx], q3);
        remove_pid(q2->q[idx], q2);
      }
    }
  }

  // Priority BOOST
  if (curr_time % S == 0 && curr_time)
  {
    q1->sz = q2->sz = q3->sz = 0;
    for (int idx = 0; idx < procs_count; idx++)
      add_pid(procs_info[idx], q1);
  }

  // Remove not active
  for (int idx = 0; idx < q1->sz; idx++)
    if (!is_active(q1->q[idx], procs_info, procs_count))
      remove_pid(q1->q[idx], q1);

  for (int idx = 0; idx < q2->sz; idx++)
    if (!is_active(q2->q[idx], procs_info, procs_count))
      remove_pid(q2->q[idx], q2);

  for (int idx = 0; idx < q3->sz; idx++)
    if (!is_active(q3->q[idx], procs_info, procs_count))
      remove_pid(q3->q[idx], q3);

  if (q1->sz)
    return rr_scheduler(q1, q1->sz, curr_time, curr_pid);

  if (q2->sz)
    return rr_scheduler(q2, q2->sz, curr_time, curr_pid);

  if (q3->sz)
    return rr_scheduler(q3, q3->sz, curr_time, curr_pid);

  return curr_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  int stcf_pid = -1;
  int stcf_time = __INT_MAX__;
  for (int idx = 0; idx < procs_count; idx++)
  {
    int time_left = process_total_time(procs_info[idx].pid) - procs_info[idx].executed_time;
    if (time_left >= 0 && time_left < stcf_time)
    {
      stcf_time = time_left;
      stcf_pid = procs_info[idx].pid;
    }
  }
  return stcf_pid;
}
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid)
{
  int sjf_pid = -1;
  int sjf_time = __INT_MAX__;
  for (int idx = 0; idx < procs_count; idx++)
  {
    int total_time = process_total_time(procs_info[idx].pid);
    if (total_time < sjf_time)
    {
      sjf_time = total_time;
      sjf_pid = procs_info[idx].pid;
    }
  }
  return sjf_pid;
  // fprintf(stderr, "SJF scheduler Not Implemented ");
  // exit(1);
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

  if (strcmp(name, "sjf") == 0)
    return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;
  if (strcmp(name, "rr") == 0)
    return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0)
    return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
