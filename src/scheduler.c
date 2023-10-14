#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "process.h"
#include "simulation.h"

// Actual global queue variables
fq_t *q0;
fq_t *q1;
fq_t *q2;

// Array containing each feedback queue ordered from high to low
fq_t *priority_array[PRIORITY_LEVELS];

// Default constructor for the feedback queue, initializes its values
// to an empty queue
static fq_t *init_fq() {
  fq_t *fq = malloc(sizeof(fq_t));

  fq->procs = NULL;
  fq->size = 0;
  fq->procs_count = 0;

  return fq;
}

// Resets all feedback queues, or initializes them depending on their
// current state
static void reset_fqs() {
  q2 = init_fq();
  q1 = init_fq();
  q0 = init_fq();
}

// Removes an element from a given feedback queue
static void fq_remove(fq_t *fq, int pid) {
  for (int i = 0; i < fq->procs_count; i++) {
    if (fq->procs[i] == pid) {
      fq->procs_count--;
      for (int j = i; j < fq->procs_count - 1; j++) {
        fq->procs[j] = fq->procs[j + 1];
      }
      return;
    }
  }
}

// Removes an element from every feedback queue it is contained
static void remove_from_all_fqs(int pid) {
  for (int level = 0; level < PRIORITY_LEVELS; level++) {
    fq_remove(priority_array[level], pid);
  }
}

// Returns the next element in the given queue
static int fq_dequeue(fq_t *fq) {
  int *procs = fq->procs;
  
  if (procs == NULL || fq->procs_count == 0) return -1;

  fq->procs_count--;
  int pid = procs[0];
  
  for (int i = 0; i < fq->procs_count; i++) {
    procs[i] = procs[i + 1];
  }

  fq->procs = procs;

  return pid;
}

// Adds a process pid to the given feedback queue, only reallocs memory if the current process
// count is greater than the max size the queue have had
static void fq_enqueue(fq_t *fq, int pid) {
  int *procs = fq->procs;
  fq->procs_count++;

  if (procs == NULL) {
    fq->size = fq->procs_count;
    procs = malloc(fq->procs_count * sizeof(int));
  }
  
  if (fq->size < fq->procs_count) {
    fq->size = fq->procs_count;
    procs = realloc(fq->procs, fq->procs_count * sizeof(int));
  }

  if (procs == NULL) return;

  procs[fq->procs_count - 1] = pid;
  fq->procs = procs;
}

// Returns true if the given pid is in any feedback queue
static int in_fq(int pid) {
  for (int level = 0; level < PRIORITY_LEVELS; level++) {
    for (int i = 0; i < priority_array[level]->procs_count; i++) {
      if (priority_array[level]->procs[i] == pid) {
        return 1;
      }
    }
  }

  return 0;
}

// Given a procs_info pointer and a pid, finds the index of the process with that pid on the pointer,
// returns -1 otherwise
static int find_proc_info_index_by_pid(proc_info_t *procs_info, int procs_count, int pid) {
  for (int i = 0; i < procs_count; i++) {
    if (procs_info[i].pid == pid) return i;
  }
  return -1;
}

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
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid) {
//   // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
//   // cambiar)


// Información que puedes obtener de un proceso
int pid = procs_info[0].pid;      // PID del proceso
int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                  // realizando una opreación IO
int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                              // ejecutado (en CPU o en I/O)


//   // También puedes usar funciones definidas en `simulation.h` para extraer
//   // información extra:
//   int duration = process_total_time(pid);

//   return -1;
// }

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  if (procs_count == 0) return -1;

  if (find_proc_info_index_by_pid(procs_info, procs_count, curr_pid) != -1) return curr_pid;

  // Finds the minimum duration process, without considering executed time
  int less_duration = INT_MAX;
  int less_duration_pid = -1;

  for (int i = 0; i < procs_count; i++) {
    int duration = process_total_time(procs_info[i].pid);

    if (duration < less_duration) {
      less_duration = duration;
      less_duration_pid = procs_info[i].pid;
    }
  }

  return less_duration_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  if (procs_count == 0) return -1;

  // Finds the minimum duration process, total_time - executed_time is used to 
  // calculate the remaining CPU time of each process
  int less_duration = INT_MAX;
  int less_duration_pid = -1;

  for (int i = 0; i < procs_count; i++) {
    int remain_duration = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;

    if (remain_duration < less_duration) {
      less_duration = remain_duration;
      less_duration_pid = procs_info[i].pid;
    }
  }

  return less_duration_pid;  
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  int pid = curr_pid;
  if (procs_count == 0) return -1;
  if (curr_pid == -1) return procs_info[0].pid;
  
  if (curr_time % ROUND_ROBIN_TIME_SLICE != 0) {    
    return pid;
  }

  // If the current time matches the slice time finds the next process to execute
  // that is not on I/O 
  for (int i = 0; i < procs_count; i++) {
    if (procs_info[i].pid == curr_pid) {
      int start_index = ++i % procs_count; 

      while (procs_info[i % procs_count].on_io) {
        i++;

        if (i % procs_count == start_index) {
          pid = procs_info[start_index].pid;
          break;
        }
      }
      pid = procs_info[i % procs_count].pid;   
      break;
    }
  }

  return pid;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid) {
  int pid = curr_pid;

  for (int i = 0; i < procs_count; i++) {
    // Adds the process to the highest priority queue if not already on a feedback queue
    if (!in_fq(procs_info[i].pid)) {
      fq_enqueue(priority_array[0], procs_info[i].pid);      
    }
  }

  // While the time matches the priority boost interval, resets the queues and re-enqueue each
  // process
  if (curr_time % MLFQ_PRIORITY_BOOST_TIME == 0) {
    reset_fqs();

    for (int i = 0; i < procs_count; i++) {
      fq_enqueue(priority_array[0], procs_info[i].pid);
    }
  }

  // Checks for the next process to execute, if the process is already ended execution
  // discards it, if the process is reading I/O, it re-enqueues it with the same priority
  if (curr_time % ROUND_ROBIN_TIME_SLICE == 0) {
    for (int level = 0; level < PRIORITY_LEVELS; level++) {
      pid = fq_dequeue(priority_array[level]);
      int proc_index = find_proc_info_index_by_pid(procs_info, procs_count, pid);

      // The queue has only one element and has finished execution,
      // go to a lower priority level
      if (pid == -1) continue;

      while (priority_array[level]->procs_count > 0) {
        if (procs_info[proc_index].on_io)
          fq_enqueue(priority_array[level], pid);
        
        if (proc_index > -1) {
          break;
        }
        
        pid = fq_dequeue(priority_array[level]);
        proc_index = find_proc_info_index_by_pid(procs_info, procs_count, pid);
      }

      // There's no valid process on current queue, go to a 
      // lower priority level
      if (pid == -1) continue;

      // Makes sure the priority descent doesn't goes out of range
      if (level == PRIORITY_LEVELS - 1) fq_enqueue(priority_array[level], pid);
      else fq_enqueue(priority_array[level++], pid);
      break;
    }    
  }
  
  return pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.
  reset_fqs();

  priority_array[0] = q2;
  priority_array[1] = q1;
  priority_array[2] = q0;

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
