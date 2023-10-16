#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>

#include "simulation.h" 

typedef struct tuple
{
  int pid;
  int own_slice_time;
}Tuple;

//cola mayor prioridad
struct Tuple queue_2[MAX_PROCESS_COUNT];
int count_queue_2 = 0;

//cola media prioridad
struct Tuple queue_1[MAX_PROCESS_COUNT];
int count_queue_1 = 0;

//cola menor prioridad
struct Tuple queue_0[MAX_PROCESS_COUNT];
int count_queue_0 = 0;

Tuple* pointer_q2 = &queue_2;
Tuple* pointer_q1 = &queue_1;
Tuple* pointer_q0 = &queue_0;


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

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int index = 0;
  int best_time = process_total_time(procs_info[index].pid);
  for (int i = 0; i < procs_count; i++)
  {
    //ejecuta 1 proceso hasta el final
    if(procs_info[i].pid == curr_pid && procs_info[i].executed_time > 0) return curr_pid; 
    
    //de los procesos pendientes (ninguno ha comenzado su ejecucion) selecciona 
    //el de menor tiempo de ejecucion
    if(process_total_time(procs_info[i].pid) < best_time)
    {
      best_time = process_total_time(procs_info[i].pid);
      index = i;
    }
  } 
  return procs_info[index].pid; 
}


int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int index = 0;
  int best_time = process_total_time(procs_info[index].pid) - procs_info[index].executed_time;

  for (int i = 1; i < procs_count; i++)
  {
    //de los procesos activos, selecciona el que le quede menor tiempo de ejecucion
     if(process_total_time(procs_info[i].pid) - procs_info[i].executed_time < best_time)
    {
      best_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      index = i;
    }
  }
  return procs_info[index].pid;
}


int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  if (procs_count == 1) return procs_info[0].pid;

  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid != curr_pid) continue;

    //slice_time = 40 
    if(curr_time % 40 != 0) return procs_info[i].pid;

    //si el proceso que actualmente ejecuta el CPU NO es el
    //ultimo en la lista de procesos activos, lo interrumpe y 
    //ejecuta el siguiente
    if(i < procs_count - 1) return procs_info[i + 1].pid;
    else break;    
  } 
  return procs_info[0].pid;
}  





int find_new_process(int pid)
{
 
  for (int i = 0; i < count_queue_2; i++)
  {
    if(pointer_q2 [i].pid != pid) continue;
    return 2;
  }

  for (int i = 0; i < count_queue_1; i++)
  {
    if(pointer_q1[i].pid != pid) continue;
    return 1;
  }

  for (int i = 0; i < count_queue_0; i++)
  {
    if(pointer_q0[i].pid != pid) continue;
    return 0;
  }
    
  return -1;  
}

int find_old_process(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i < procs_count; i++)
  {
    if(procs_info[i].pid == pid) return i;
  }
  return -1;  
}

void priority_boost()
{
  for (int i = count_queue_2 - 1; i >= 0; i--)
  {    
    pointer_q2[i + count_queue_0 + count_queue_1] = pointer_q2[i];
  }

  for (int i = 0; i < count_queue_0; i++)
  {
    pointer_q2[i] = pointer_q0[i];
    pointer_q0[i].own_slice_time = 0;
    pointer_q0[i].pid = 0;
  }

  for (int i = 0; i < count_queue_1; i++)
  {
    pointer_q2[i + count_queue_0] = pointer_q1[i];
    pointer_q1[i].own_slice_time = 0;
    pointer_q1[i].pid = 0;
  }

  count_queue_0 = 0;
  count_queue_1 = 0;
}

int get_position(Tuple *queue, int count_queue, int pid)
{

  for (int i = 0; i < count_queue; i++)
  {
    if(queue[i].pid == pid) return i;  
  }

}

void delete_procs_from_queue(Tuple *queue, int count_queue, int pos)
{

  for (int i = pos; i < count_queue - 1; i++)
  {
    queue[i] = queue[i + 1];
  }
  if(queue == pointer_q2) count_queue_2 -= 1;
  else if (queue == pointer_q1) count_queue_1 -= 1;
  
}




int rr_for_mlfq_scheduler(proc_info_t *procs_info, int procs_count, Tuple *queue, int queue_count)
{
  for (int i = 0; i < queue_count; i++)
  {
    if(queue[i].own_slice_time == 40)
    {

      if(count_queue_1 == 0 && count_queue_2 == 0) return queue[i].pid;


      if(queue == pointer_q2) 
      {
        pointer_q1[count_queue_1] = queue[i];
        pointer_q1[count_queue_1].own_slice_time = 0;
        pointer_q1[count_queue_1].pid = queue[i].pid;
        count_queue_1 += 1;
        delete_procs_from_queue(queue, queue_count, i);
      }

        if(queue == pointer_q1)
      { 
        pointer_q0[count_queue_0] = queue[i];
        pointer_q0[count_queue_0].own_slice_time = 0;
        pointer_q0[count_queue_0].pid = queue[i].pid;
        count_queue_0 += 1;
        delete_procs_from_queue(queue, queue_count, i);
      } 
      continue;      
    }

    for (int j = 0; j < procs_count; j++)
    {
      if(procs_info[j].pid == queue[i].pid)
      {
        if(!procs_info[j].on_io)
        {
          queue[i].own_slice_time += 10;
          return queue[i].pid;
        }
      }
    }
  }
  
  queue[0].own_slice_time += 10;
  return queue[0].pid;
} 

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{

  //se eliminan de las colas los procesos que ya terminaron de ejecutarse

  for (int i = 0; i < count_queue_2; i++)
  {
    if(find_old_process(procs_info, procs_count, pointer_q2[i].pid) == -1)
    {
      delete_procs_from_queue(pointer_q2, count_queue_2, i);
    }
  }

  for (int i = 0; i < count_queue_1; i++)
  {
    if(find_old_process(procs_info, procs_count, pointer_q1[i].pid) == -1)
    {
      delete_procs_from_queue(pointer_q1, count_queue_1, i);
    }
  }

  for (int i = 0; i < count_queue_0; i++)
  {
    if(find_old_process(procs_info, procs_count, pointer_q0[i].pid) == -1)
    {
      delete_procs_from_queue(pointer_q0, count_queue_0, i);
    }
  }

  //se incorporan a la cola de mayor prioridad los procesos nuevos (si hay)
  for (int i = 0; i < procs_count; i++)
  {
    if(find_new_process(procs_info[i].pid) == -1)
    {
      for (int i = count_queue_2 - 1; i >= 0; i--)
      {
        pointer_q2[i + 1] = pointer_q2[i];
      }
      pointer_q2[0].pid = procs_info[i].pid;
      pointer_q2[0].own_slice_time = 0;
      count_queue_2 += 1;      
    }
  }


  // boost_time = 120
  if(curr_time % 120 == 0)
  {
    priority_boost;
    return rr_for_mlfq_scheduler(pointer_q2, count_queue_2, curr_time, curr_pid);    
  }
  if(count_queue_2 != 0) return rr_for_mlfq_scheduler(procs_info, procs_count, pointer_q2, count_queue_2);
  if(count_queue_1 != 0) return rr_for_mlfq_scheduler(procs_info, procs_count, pointer_q1, count_queue_1);
  if(count_queue_0 != 0) return rr_for_mlfq_scheduler(procs_info, procs_count, pointer_q0, count_queue_0);

  return -1;
  
  
}
      






// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid) {
//   // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
//   // cambiar)

//   // Información que puedes obtener de un proceso
//   int pid = procs_info[0].pid;      // PID del proceso
//   int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
//                                     // realizando una opreación IO
//   int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
//                                                 // ejecutado (en CPU o en I/O)

//   // También puedes usar funciones definidas en `simulation.h` para extraer
//   // información extra:
//   int duration = process_total_time(pid);

//   return -1;
// }

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

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
