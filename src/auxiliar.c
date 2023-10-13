#include "auxiliar.h"
void depurate_mlfq(mlfq _mlfq, proc_info_t *procs_info, int procs_count)
{
  int store_pids_count = 0;
  int *stored_pids = mlfq_processes(_mlfq, &store_pids_count);

  for (int i = 0; i < store_pids_count; i++)
  {
    bool is_present = FALSE;
    for (int j = 0; j < procs_count; j++)
      if ((is_present = (stored_pids[i] == procs_info[j].pid)))
        break;
    if (!is_present)
      mlfq_remove(_mlfq, stored_pids[i]);
  }
}

bool check_existence(proc_info_t *procs_info, int procs_count, int pid)
{
  bool is_present = FALSE;
  for (int i = 0; i < procs_count; i++)
    if ((is_present = (procs_info[i].pid == pid)))
      break;
  return is_present;
}

typedef int (*get_time_action)(proc_info_t *);

int sjf_get_time(proc_info_t *process)
{
  return process_total_time(process->pid);
}

int stcf_get_time(proc_info_t *process)
{
  return process_total_time(process->pid) - process->executed_time;
}

// Finds the shortest process among the active ones
int get_shortest_job(proc_info_t *procs_info, int procs_count, get_time_action get_time)
{
  int min_pid = procs_info[0].pid;
  int min_time = get_time(&procs_info[0]);
  // int min_time = process_total_time(min_pid) - procs_info[0].executed_time;

  for (int i = 1; i < procs_count; i++)
  {
    int temp_pid = procs_info[i].pid;
    // int temp_time = process_total_time(temp_pid) - procs_info[i].executed_time;
    int temp_time = get_time(&procs_info[i]);

    if (temp_time < min_time)
    {
      min_time = temp_time;
      min_pid = temp_pid;
    }
  }
  return min_pid;
}