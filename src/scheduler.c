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

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Si no hay procesos en la cola, no hay nada que ejecutar
  if (procs_count == 0)
  {
    return -1;
  }

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

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Si no hay procesos en la cola, no hay nada que ejecutar
  if (procs_count == 0)
  {
    return -1;
  }
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

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid)
{
  // Si no hay procesos en la cola, no hay nada que ejecutar
  if (procs_count == 0)
  {
    return -1;
  }
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
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
