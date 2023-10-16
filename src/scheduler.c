#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

// Declaración de la cola de procesos para MLFQ
struct Process
{
  int pid;
  int executed_time;
};

// Invariantes de las colas
int time_slice = 50;
int priority_boost = 500;

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
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

/*int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
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
}*/

//------------------------------SHORTEST JOB FIRST------------------------------
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Si hay un proceso en marcha, termínalo siempre que no esté en i/o
  if (curr_pid != -1)
  {
    int process_location = 0;
    while (curr_pid != procs_info[process_location].pid)
    {
      process_location++;
    }
    // Aquí tenemos la posición de nuestro proceso
    // Ahora vemos si está en i/o o no, y si no está, lo ejecutamos
    if (!(procs_info[process_location].on_io))
    {
      return curr_pid;
    }
  }
  // Si no hay un proceso en marcha (o el actual está en i/o) y hay procesos en cola, seleccionar el más corto de los que no estén en i/o
  int i = 0;
  // Guardamos nuestro primer proceso que no esté en i/o
  while (i < procs_count && procs_info[i].on_io)
  {
    i++;
  }
  // Si pasaste por todos los procesos y todos están en i/o, no ejecutes ninguno
  if (i == procs_count)
  {
    return -1;
  }
  // De lo contrario, guardamos el primero de estos procesos y empezamos a comparar sus tiempos
  int process = procs_info[i].pid;
  while (i < procs_count)
  {
    if ((process_total_time(process) > process_total_time(procs_info[i].pid)) && !(procs_info[i].on_io))
    {
      process = procs_info[i].pid;
    }
    i++;
  }
  return process;
}

//------------------------------SHORTEST TIME TO COMPLETION FIRST------------------------------
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  int i = 0;
  // Guardamos nuestro primer proceso que no esté en i/o
  while (i < procs_count && procs_info[i].on_io)
  {
    i++;
  }
  // Si pasaste por todos los procesos y todos están en i/o, no ejecutes ninguno
  if (i == procs_count)
  {
    return -1;
  }
  // De lo contrario, guardamos el primero de estos procesos y empezamos a comparar sus tiempos
  // En este caso nos quedaremos con el que menos tiempo le quede para terminar su ejecución
  int process = procs_info[i].pid;
  int process_remaining_time = process_total_time(process) - procs_info[i].executed_time;
  while (i < procs_count)
  {
    if ((process_remaining_time > process_total_time(procs_info[i].pid) - procs_info[i].executed_time) && (!procs_info[i].on_io))
    {
      process = procs_info[i].pid;
      process_remaining_time = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    }
    i++;
  }
  return process;
}

//------------------------------ROUND ROBIN------------------------------
// Este Round Robin usa como time slice el mismo tiempo del time interrupt
int rr_mark = 0;
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  rr_mark++;
  if (rr_mark >= procs_count)
  {
    rr_mark = 0;
  }
  return procs_info[rr_mark].pid;
}

//------------------------------PRIORITY ROUND ROBIN------------------------------
int priority_rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Siempre ejecutar el primer proceso de la lista con menor tiempo ejecutado
  int i = 0;
  // Guardamos nuestro primer proceso que no esté en i/o
  while (i < procs_count && procs_info[i].on_io)
  {
    i++;
  }
  // Si pasaste por todos los procesos y todos están en i/o, no ejecutes ninguno
  if (i == procs_count)
  {
    return -1;
  }
  // De lo contrario, guardamos el primero de estos procesos y empezamos a comparar sus tiempos
  int process = procs_info[i].pid;
  int process_exec_time = procs_info[i].executed_time;
  while (i < procs_count)
  {
    if (procs_info[i].executed_time < process_exec_time && procs_info[i].pid != curr_pid && (!procs_info[i].on_io)) // Garantizando que no se repita el proceso
    {
      process = procs_info[i].pid;
      process_exec_time = procs_info[i].executed_time;
    }
    i++;
  }
  return process;
}

//------------------------------MLFQ------------------------------
// Métodos usados
int Get_Position(proc_info_t *procs_info, int procs_count, int pid)
{
  int pos = -1;
  for (int i = 0; i < procs_count; i++)
  {
    if (procs_info[i].pid == pid)
    {
      pos = i;
      break;
    }
  }
  return pos;
}

int Get_Position_in_List(struct Process *list, int procs_count, int pid)
{
  int pos = -1;
  for (int i = 0; i < procs_count; i++)
  {
    if (list[i].pid == pid)
    {
      pos = i;
      break;
    }
  }
  return pos;
}

// Eliminar un proceso en una cola
void Eliminate_Process(struct Process *procs_info, int procs_count, int process_pid)
{
  for (int i = process_pid; i < procs_count - 1; i++)
  {
    procs_info[i] = procs_info[i + 1];
  }
}

// Elegir el proceso a ejecutar en una cola
int Get_Next_PID(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid, struct Process *list, int list_procs_count, int *list_init, int *last_pos)
{
  int pos;
  pos = Get_Position(procs_info, procs_count, list[*last_pos].pid);
  //Verificando si el proceso en cuestión cumple con los requisitos para ser devuelto (no i/o, tiempo correcto, está en cola)
  if (pos != -1 && !procs_info[pos].on_io && (curr_time - *list_init < time_slice))
  {
    return list[*last_pos].pid;
  }
  //De lo contrario, busca el próximo proceso a ejecutar por Round Robin, si no hay retorna -1.
  else
  {
    int counter = 0;
    while(counter<list_procs_count)
    {
      *last_pos=*last_pos+1;
      if (*last_pos >= list_procs_count)
      {
        *last_pos = 0;
      }
      pos = Get_Position(procs_info, procs_count, list[*last_pos].pid);
      if (!procs_info[pos].on_io)
      {
        *list_init = curr_time;
        return list[*last_pos].pid;
      }
      counter++;
    }
    return -1;
  }
}



// Cola de prioridades
struct Process high_priority_list[MAX_PROCESS_COUNT];
struct Process medium_priority_list[MAX_PROCESS_COUNT];
struct Process low_priority_list[MAX_PROCESS_COUNT];
// Elementos de las colas
int h_init = 0;
int h_position = 0;
int h_procs_count = 0;

int m_init = 0;
int m_position = 0;
int m_procs_count = 0;

int l_init = 0;
int l_position = 0;
int l_procs_count = 0;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Principalmente hay que actualizar las listas de prioridades
  // Eliminando de las colas procesos finalizados
  for (int i = 0; i < h_procs_count; i++)
  {
    int procs_pos = Get_Position(procs_info, procs_count, high_priority_list[i].pid);
    if (procs_pos == -1)
    {
      Eliminate_Process(high_priority_list, h_procs_count, i);
      h_procs_count--;
    }
  }
  for (int i = 0; i < m_procs_count; i++)
  {
    int procs_pos = Get_Position(procs_info, procs_count, medium_priority_list[i].pid);
    if (procs_pos == -1)
    {
      Eliminate_Process(medium_priority_list, m_procs_count, i);
      m_procs_count--;
    }
  }
  for (int i = 0; i < l_procs_count; i++)
  {
    int procs_pos = Get_Position(procs_info, procs_count, low_priority_list[i].pid);
    if (procs_pos == -1)
    {
      Eliminate_Process(low_priority_list, l_procs_count, i);
      l_procs_count--;
    }
  }
  // En este punto ya las colas están actualizadas.
  // Luego, si se ejecutó un proceso que no ha terminado, lo buscamos en las colas
  if (curr_pid != -1)
  {
    int process_position = Get_Position_in_List(high_priority_list, h_procs_count, curr_pid);
    // Revisando en la lista de mayor prioridad
    if (process_position != -1)
    {
      // Aumentando el tiempo ejecutado del proceso
      high_priority_list[process_position].executed_time += 10;
      // Si el proceso cumplió con todo su tiempo disponible, bajar su prioridad
      if (high_priority_list[process_position].executed_time >= time_slice)
      {
        medium_priority_list[m_procs_count].pid = high_priority_list[process_position].pid;
        medium_priority_list[m_procs_count].executed_time = 0;
        m_procs_count++;
        // Eliminando proceso de la lista
        Eliminate_Process(high_priority_list, h_procs_count, process_position);
        h_procs_count--;
      }
    }
    // Revisando en la lista de prioridad media
    else
    {
      int process_position = Get_Position_in_List(high_priority_list, h_procs_count, curr_pid);
      if (process_position != -1)
      {
        // Aumentando el tiempo ejecutado del proceso
        medium_priority_list[process_position].executed_time += 10;
        if (medium_priority_list[process_position].executed_time >= time_slice)
        {
          low_priority_list[l_procs_count].pid = medium_priority_list[process_position].pid;
          low_priority_list[l_procs_count].executed_time = 0;
          l_procs_count++;
          // Eliminando proceso de la lista
          Eliminate_Process(medium_priority_list, m_procs_count, process_position);
          m_procs_count--;
        }
      }
    }
    // En la última cola no es necesario revisar, pues no hay que bajar más prioridad
  }
  // Ahora debemos verificar si ejecutar o no un PRiority Boost
  if (curr_time % priority_boost == 0)
  {
    // Debemos mover todos los procesos a la primera cola
    h_procs_count = 0;
    m_procs_count = 0;
    l_procs_count = 0;
    for (int i = 0; i < procs_count; i++)
    {
      high_priority_list[i].pid = procs_info[i].pid;
      high_priority_list[i].executed_time = 0;
      h_procs_count++;
    }
    // La posición se pone en 0 para empezar de 0 a analizar la lista
    h_position = 0;
  }
  //De lo contrario, agregamos cualquier proceso que pudo haber llegado y no se encuentra en las colas
  else
  {
    for (int i = 0; i < procs_count; i++)
    {
      if (Get_Position_in_List(high_priority_list, h_procs_count, procs_info[i].pid)==-1  && Get_Position_in_List(medium_priority_list, m_procs_count, procs_info[i].pid) ==-1 && Get_Position_in_List(low_priority_list,l_procs_count,procs_info[i].pid ==-1))
      {
        high_priority_list[h_procs_count].pid = procs_info[i].pid;
        high_priority_list[h_procs_count].executed_time = 0;
        h_procs_count++;
      }
    }
  }
  //Finalmente queda elegir el próximo proceso a ejecutar
  if (h_procs_count > 0)
  {
    int pid = Get_Next_PID(procs_info, procs_count, curr_time, curr_pid, high_priority_list, h_procs_count, &h_position, &h_init);
    if (pid != -1)
    {
      return pid;
    }
  }
  if (m_procs_count > 0)
  {
    int pid = Get_Next_PID(procs_info, procs_count, curr_time, curr_pid, medium_priority_list, m_procs_count, &m_position, &m_init);
    if (pid != -1)
    {
      return pid;
    }
  }
  if (l_procs_count > 0)
  {
    int pid = Get_Next_PID(procs_info, procs_count, curr_time, curr_pid, low_priority_list, l_procs_count, &l_position, &l_init);
    if (pid != -1)
    {
      return pid;
    }
  }
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
    return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0)
    return *stcf_scheduler;
  if (strcmp(name, "rr") == 0)
    return *rr_scheduler;
  if (strcmp(name, "prr") == 0)
    return *priority_rr_scheduler;
  if (strcmp(name, "mlfq") == 0)
    return *mlfq_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
