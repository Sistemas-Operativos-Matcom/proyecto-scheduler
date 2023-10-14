#include "simulation.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "process.h"

#define TIMER_INTERRUPT_PERIOD_MS 10

static int64_t ms_time()
{
  struct timespec now;
  timespec_get(&now, TIME_UTC);
  return ((int64_t)now.tv_sec) * 1000 + ((int64_t)now.tv_nsec) / 1000000;
}

static simulation_t *g_sim;

static simulation_t new_simulation(process_t *processes, int process_count)
{
  simulation_t sim = {
      (proc_execution_t *)malloc(process_count * sizeof(proc_execution_t)),
      process_count,
      0,
      -1,
  };

  for (int i = 0; i < process_count; i++)
  {
    sim.procs_exec_info[i].process = processes[i];
    sim.procs_exec_info[i].pid = i;
    sim.procs_exec_info[i].executed_time = 0;
    sim.procs_exec_info[i].state = NOT_ARRIVED;
  }

  return sim;
}

static process_t *get_process_by_id(int pid)
{
  assert(g_sim != NULL);
  assert(pid < g_sim->processes_count);
  return &(g_sim->procs_exec_info[pid].process);
}

static void fill_proc_info(proc_execution_t *proc_exec,
                           proc_info_t *proc_info)
{
  proc_info->pid = proc_exec->pid;
  proc_info->on_io = proc_exec->state == ON_IO;
  proc_info->executed_time = proc_exec->executed_time;
}

static int update_process(proc_execution_t *process_exec, int curr_time)
{
  if (process_exec->executed_time == 0)
  {
    process_exec->response_time = curr_time - process_exec->process.arrive_time;
  }
  process_exec->executed_time += TIMER_INTERRUPT_PERIOD_MS;
  int accu_time = 0;
  int state = RUNNABLE;
  for (int i = 0; i < process_exec->process.ev_count; i++)
  {
    state = i % 2 ? ON_IO : RUNNABLE;
    accu_time += process_exec->process.event_durations_msecs[i];
    if (process_exec->executed_time <= accu_time)
    {
      return process_exec->state = state;
    }
  }
  process_exec->turnaround = curr_time - process_exec->process.arrive_time;
  return process_exec->state = ENDED;
}

static void show_sim_state()
{
  printf("[%8d]", g_sim->curr_time);
  char indicator = '.';
  proc_execution_t exec_info;
  for (int i = 0; i < g_sim->processes_count; i++)
  {
    if (i % 5 == 0)
      printf(" ");
    indicator = '.';
    exec_info = g_sim->procs_exec_info[i];
    if (exec_info.state == RUNNABLE)
    {
      indicator = g_sim->curr_proc_pid == exec_info.pid ? 'O' : '=';
    }
    else if (exec_info.state == ON_IO)
    {
      indicator = 'I';
    }
    printf("%c", indicator);
  }
  printf("\n");
}

static void show_sim_final_stats(double total_sim_time)
{
  float mean_ta = 0;
  int max_ta = 0;
  int min_ta = -1;

  float mean_rt = 0;
  int max_rt = 0;
  int min_rt = -1;

  for (int i = 0; i < g_sim->processes_count; i++)
  {
    int ta = g_sim->procs_exec_info[i].turnaround;
    int rt = g_sim->procs_exec_info[i].response_time;

    mean_ta += ta;
    max_ta = ta > max_ta ? ta : max_ta;
    min_ta = ta < min_ta || min_ta == -1 ? ta : min_ta;

    mean_rt += rt;
    max_rt = rt > max_rt ? rt : max_rt;
    min_rt = rt < min_rt || min_rt == -1 ? rt : min_rt;
  }

  mean_ta /= g_sim->processes_count;
  mean_rt /= g_sim->processes_count;

  printf("Simulation took: %e s\n\n", total_sim_time);

  printf("Turnaround:\n");
  printf("  Mean time: %.3f\n", mean_ta);
  printf("  Min-Max:   %d - %d\n\n", min_ta, max_ta);

  printf("Response time:\n");
  printf("  Mean time: %.3f\n", mean_rt);
  printf("  Min-Max:   %d - %d\n\n", min_rt, max_rt);
}

void start_new_simulation(process_t *processes, int process_count,
                          schedule_action_t scheduler_action, int config)
{
  simulation_t sim = new_simulation(processes, process_count);
  g_sim = &sim;

  int show_graph = config & OPT_GRAPH;
  int real_time = config & OTP_REAL_TIME;

  // Cantidad de procesos activos (ya arrivaron y no han finalizado)
  int active_processes = 0;

  // Cantidad de procesos que terminaron su ejecución
  int ended_processes = 0;

  // Información de la ejecución de los procesos activos.
  proc_info_t *procs_info =
      (proc_info_t *)malloc(process_count * sizeof(proc_info_t));

  // Registra si la simulación finalizó
  int ended_sim = 0;

  // Contiene los pids de los procesos que se encuentran haciendo IO request.
  // La forma en la que se procesan los IO request es aleatoria (por
  // simplicidad). Por tanto, en cada timer-interrupt se consumirá el tiempo de
  // procesamiento de uno de estos procesos de forma aleatoria.
  int *on_io = (int *)malloc(process_count * sizeof(int));
  int on_io_count = 0;

  // Esta variable se usará para guardar un índice aleatorio para obtener
  // que proceso en la cola de IO (si existen) consumirá su tiempo
  int random_idx;

  // Registra el tiempo en el que comenzó la simulación
  int64_t start_sim_time = ms_time();

  // Mientras no hayan finalizados todos los procesos se mantiene corriendo la
  // simulación
  while (!ended_sim)
  {
    int next_sim_time = g_sim->curr_time + TIMER_INTERRUPT_PERIOD_MS;

    // Comprobar si arrivaron procesos
    for (int i = 0; i < g_sim->processes_count; i++)
    {
      if (g_sim->curr_time >= g_sim->procs_exec_info[i].process.arrive_time &&
          g_sim->procs_exec_info[i].state == NOT_ARRIVED)
      {
        g_sim->procs_exec_info[i].state = RUNNABLE;
      }
    }

    if (show_graph)
      show_sim_state();

    // Se recalculan los procesos activos (y los que se encuentra haciendo
    // operaciones IO)
    active_processes = 0;
    on_io_count = 0;
    for (int i = 0; i < process_count; i++)
    {
      // Un proceso está activo si ya arrivó y no ha terminado.
      //  - RUNNABLE (en espera o en CPU)
      //  - ON_IO (haciendo una operación IO)
      if (g_sim->procs_exec_info[i].state & (RUNNABLE | ON_IO))
      {
        // Actualiza la información del i-ésimo proceso activo y a su vez el
        // contador (de esta forma no hay que alocar memoria nueva en cada
        // timer-interrupt).
        fill_proc_info(&(g_sim->procs_exec_info[i]),
                       &procs_info[active_processes++]);

        // Si el proceso se encuentra realizando una operación IO se agrega el
        // pid a la lista de procesos en IO y se actualiza el contador.
        if (g_sim->procs_exec_info[i].state == ON_IO)
        {
          on_io[on_io_count++] = g_sim->procs_exec_info[i].pid;
        }
      }
    }

    if (active_processes)
    {
      // Pid del proceso que va a ejecutarse en CPU durante el siguiente
      // intervalo de timer-interrupt según el scheduler (puede ser el mismo).
      int next_active_pid = scheduler_action(
          procs_info, active_processes, g_sim->curr_time, g_sim->curr_proc_pid);

      // Comprueba que el proceso a ejecutar es un proceso activo.
      if (next_active_pid != -1)
      {
        int is_correct = 0;
        for (int i = 0; i < active_processes; i++)
        {
          if ((is_correct = (procs_info[i].pid == next_active_pid)))
            break;
        }
        if (!is_correct)
        {
          fprintf(stderr, "Invalid next pid: %d\n", next_active_pid);
          exit(1);
        }

        // Actualiza la ejecución del proceso (si no está realizando una
        // operación IO)
        if (g_sim->procs_exec_info[next_active_pid].state == RUNNABLE)
        {
          if (update_process(&g_sim->procs_exec_info[next_active_pid],
                             next_sim_time) == ENDED)
          {
            g_sim->curr_proc_pid = -1;
            ended_processes++;
          }
          else
          {
            g_sim->curr_proc_pid = next_active_pid;
          }
        }
      }
    }

    // Actualizar un proceso que esté realizando IO de forma aleatoria.
    if (on_io_count)
    {
      random_idx = ms_time() % on_io_count;
      if (update_process(&g_sim->procs_exec_info[on_io[random_idx]],
                         next_sim_time) == ENDED)
      {
        ended_processes++;
        if (g_sim->curr_proc_pid == on_io[random_idx])
        {
          g_sim->curr_proc_pid = -1;
        }
      }
    }

    // Actualiza el tiempo de la simulación
    g_sim->curr_time = next_sim_time;

    // Si se está simulando en tiempo real espera un tiempo igual al del
    // timer-interrupt
    if (real_time)
      usleep(TIMER_INTERRUPT_PERIOD_MS * 1000);

    // Comprueba si todos los procesos terminaron
    ended_sim = ended_processes == process_count;
  }

  int64_t final_sim_time = ms_time();
  show_sim_final_stats((final_sim_time - start_sim_time) / 1000.0);
}

int process_total_time(int pid)
{
  assert(g_sim != NULL);
  process_t *process = get_process_by_id(pid);
  return process->total_duration_msecs;
}
