#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
#include "structures.h"
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


int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  if(curr_pid==-1)
  {
    // Si el proceso termina pues decidimos el proximo proceso.
    int min = 0;
    for(int i=0; i<procs_count;i++)
    {
      // Por cada proceso se guarda del indice del proceso de menor 
      // duracion.
      min = (process_total_time(procs_info[min].pid)<process_total_time(procs_info[i].pid))?min:i;
    }
    return procs_info[min].pid;
  }
  // Si el proceso no ha terminado pues se mantiene en ejecucion.
  return curr_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  int min = 0;
  for(int i=0; i<procs_count;i++)
  {
    // Este es analogo al sjf pero con la diferencia de que
    // se guarda el minimo de quien le quede menos tiempo para 
    // terminar.
    int temp1 = process_total_time(procs_info[min].pid)-procs_info[min].executed_time;
    int temp2 = process_total_time(procs_info[i].pid)-procs_info[i].executed_time;
    min = (temp1<temp2)?min:i;
  }
  // En todos los time interrupt se vuelve a obtener el minimo.
  return procs_info[min].pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  static int round_length = 0;
  static int current_proc = -1;
  if(round_length == 0 || curr_pid == -1)
  {
    // Si se acabo la ronda o se acabo el proceso, se reinicia 
    // el round_length y se ejecuta el siguiente proceso.
    current_proc++;
    round_length = 5;
    return procs_info[current_proc%procs_count].pid;
  }
  // En cada ejecucion se disminuye el round_length
  // hasta que eventualmente se vuelva 0.
  round_length--;
  return curr_pid;
}

// Colas de prioridad.
queue_t *q1;
queue_t *q2;

// Metodos auxiliares que usa mlfq.
int find_in_procs(int pid, proc_info_t* arr, int count)
{
  for(int i = 0; i < count; i++)
  {
    if(arr[i].pid == pid)
      return i;
  }
  return -2;
}
int find(int pid, int* arr, int count)
{
  for(int i = 0; i < count; i++)
  {
    if(arr[i] == pid)
      return i;
  }
  return -2;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid)
{
  // Reglas de mlfq:
  // 1-Si pr(A) > pr(B) ejecutas A
  // 2-Si pr(A) == pr(B) ejecutas rr sobre tal cola.
  // 3-Cuando un proceso consuma el slice_time de la cola actual, baja su prioridad.
  // 4-Cada cierto tiempo todos los proceso se proporcionaran a la cola de mayor prioridad
  // 5-Llegada de un proceso es en la cola de mayor prioridad.
  
  // Agregando procesos nuevos en las colas.
  for(int i = 0; i < procs_count; i++)
  {
    // Chequeando que el proceso no este en ninguna de las dos colas.
    proc_info_t p = procs_info[i]; 
    if(find(p.pid,q1->arr,q1->count) == -2 && find(p.pid,q2->arr,q2->count) == -2)
    {
      // Los procesos nuevos van a la cola de mayor prioridad, cumpliendo 
      // la regla 5.
      push(q1,p.pid);
    }
  } 

  // Actualizando la informacion que hay en las colas.
  while(q1->count > 0 && find_in_procs(q1->arr[0],procs_info, procs_count) == -2)
  {
    pop(q1); 
  }
  while(q2->count > 0 && find_in_procs(q2->arr[0],procs_info, procs_count) == -2)
  {
    pop(q2); 
  }
  // En este punto las colas deben tener en su "front" informacion
  // valida.

  // Ya tenemos toda la informacion que necesitamos, 
  // ahora solo hay que haces lo que manda mlfq
  
  // En pos de evitar "starvation" primero revisemos si estamos en un 
  // intervalo donde hagamos un priority boost.
  // Haciendo esto cumplimos con la regla 4 de mlfq.
  static int priority_boost_slice = 10;
  if(priority_boost_slice <= 0)
  {
    while(q2->count>0)
    {
      int p = q2->arr[0];
      if(find_in_procs(p,procs_info,procs_count) != -2)
        push(q1,p);
      pop(q2);
    }
    priority_boost_slice = 10;
  }
  else 
  {
    priority_boost_slice--;
  }
  // Para cumplir la regla 1 revisamos primero si hay procesos por
  // ejecutar en la cola de mayor prioridad.
  static int curr_q = 1;
  static int q1_time_slice = 5;
  static int q2_time_slice = 5;
  // Si hay elementos por ejecutar y ademas es la cola que le toca
  // la ejecucion entonces se revisan las demas propiedades
  // que debe cumplir para ejecutar el proceso.
  if(q1->count > 0 && curr_q == 1)
  {
    for(int i= 0; i < q1->count; i++)
    {
      // Buscamos el primer proceso de la cola de mayor 
      // prioridad que no este haciendo "io".
      int index = find_in_procs(q1->arr[i],procs_info,procs_count);
      if(index != -2 && procs_info[index].on_io == 0)
      {
        // Si el proceso no esta en "io" lo ejecutamos.
        int pid = q1->arr[i];
        q1_time_slice--;
        if(q1_time_slice <= 0)
        {
          // El time_slice acabo por tanto debemos bajar el proceso
          // de prioridad para cumplir la regla 3.
          q1_time_slice = 5;
          push(q2,pid);
          pop(q1);
        }
        return pid;
      }
    }  
  }
  else 
  {
    // Si no es posible ejecutar ningun proceso de la cola de mayor
    // prioridad pues debemos buscar en las siguiente cola.
    curr_q = 2;
  }

  // Ejecucion de la cola de menor prioridad.
  if(q2->count > 0 && curr_q == 2)
  {
    q2_time_slice --;
    if(q2_time_slice <= 0)
    {
      // Si se acaba el time slice pues se reinicia y se cambia de cola.
      q2_time_slice = 5;
      curr_q = 1;
    }
    return q2->arr[0];
  }
  
  // En caso de que todos los procesos esten "io"
  // pues deveolvemos el primer proceso quenos 
  // encontramos.
  if(q1->count > 0)
    return q1->arr[0];
  if(q2->count > 0)
    return q2->arr[0];

  // Algun problema paso...
  return -403;
}


// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0)
  {
    // Inicializando las colas que usara mlfq.
    q1 = build_queue();
    q2 = build_queue();
    return *mlfq_scheduler;
  }  
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
