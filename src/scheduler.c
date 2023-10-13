#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"
static int prioridad[1000000];//este array simula la cola en q esta cada proceso
                              //el pid de cada proceso es su indice en el array
                              //y el valor es la cola en la q esta

static int tiempo[1000000]; //similiar al array prioridad solo q en este se 
                            // se guarda el tiempo q cada proceso lleva con
                            //esa prioridad

static int indice; //guarda el indice del ultimo proceso q se ejecuto

static int s; //tiemp en el q todos los procesos suben a la mayor prioridad
static int prioridad_actual; // prioridad del proceso actual
static int global = 10; //teimpo q se lleva ejecutando un proceso en rr

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
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int i;
  unsigned int a;
  int c;
  a = -1;
  c = -1;
  //verificamos que el proceso q se estaba ejecutando haya terminado
  if(curr_pid == -1)
  {
    //si termino se busca entonces el de menor tiempo de ejecucion que no
    //este haciendo io
    for(i = 0; i < procs_count;i++)
    {
      if(process_total_time(procs_info[i].pid) < a && !procs_info[i].on_io)
      {
        a = process_total_time(procs_info[i].pid);
        c = procs_info[i].pid;
      }
    }
    return c;
  }
  else
  {
    //si el proceso no ha terminado entonces se vuelve a retornar
    return curr_pid;
  }
}
//busca de todos los procesos q no estan haciendo io el q menos tiempo de 
//ejecucion le quede
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int i;
  unsigned int a;
  int c;
  a = -1;
  c = -1;
  for(i = 0; i < procs_count;i++)
  {
    if(process_total_time(procs_info[i].pid)- procs_info[i].executed_time < a && !procs_info[i].on_io)
    {
      a = process_total_time(procs_info[i].pid)- procs_info[i].executed_time;
      c = procs_info[i].pid;
    }
  }
  return c;
}
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  global = global + 10;
  //si el proceso termino o consumio un tiempo igual al slice time(40 en este caso)
  //se retorna el siguiente que no este haciendo io
  if(curr_pid == -1 || global == 40)
  {
    global = 0;
    if(curr_pid != -1){
    indice++;}

    if(indice >= procs_count)
    {
      indice = 0;
    }
    if(procs_info[indice].on_io)
    {
      int j;
      for(j = indice; j < procs_count; j++)
      {
        if(!procs_info[j].on_io)
        {
          return procs_info[j].pid;
        }
      }
    }
    return procs_info[indice].pid;
  }
  else
  {
    //si el proceso no ha terminado, no ha consumido el slice time
    //todavia pero esta haciendo io, entonces se ejecuta el siguiente que
    //no este haciendo io 
    if(procs_info[indice].on_io)
    {
      int j;
      for(j = indice; j < procs_count; j++)
      {
        if(!procs_info[j].on_io)
        {
          return procs_info[j].pid;
        }
      }
    }
    //si no esta haciendo io se retorna el mismo
    return curr_pid;
  }
}
//inicialmente como no ha arrivado ningun proceso se inicializan con
//prioridad -1
void actualizar()
{
  int i;
  for(i = 0; i < 1000000; i++)
  {
    prioridad[i] = -1;
    tiempo[i] = 0;
  }
}
//verifica que el pid que se va a devolver es un pid valido
int findpid(proc_info_t *procs_info, int procs_count, int pid)
{
  int i;
  for(i = 0; i < procs_count; i++)
  {
    if(procs_info[i].pid == pid)
    {
      return 1;
    }
  }
  return -1;
}
//devuelve segun la prioridad especificada el siguiente proceso a ejecutar
int calcula(int ini, int fin, int comp, proc_info_t *procs_info, int procs_count, int c)
{
  int i;
  int a;
  for(i = ini; i < fin; i++)
  {
    a = findpid(procs_info, procs_count,procs_info[i].pid);
    if(prioridad[procs_info[i].pid] == comp && procs_info[i].on_io == 0 && a ==1)
    {
      prioridad_actual = prioridad[procs_info[i].pid];
      if(c == 0){
        indice = i;
      }
      return procs_info[i].pid;
    }
  }
  return -4;
}
//si proceso q se estaba ejecutando ya termino busca el siguiente
//de mayor prioridad
int proceso_terminado(proc_info_t *procs_info, int procs_count)
{
  int e;
  if(prioridad_actual == 1){
      e = calcula(indice, procs_count, 1,procs_info,procs_count, 0);
      if(e != -4)
      {
        return e;
      }
      e = calcula(0, indice, 1, procs_info,procs_count, 0);
      if(e != -4)
      {
        return e;
      }
      e = calcula(0, procs_count, 2, procs_info, procs_count, 0);
      if(e != -4)
      {
        return e;
      }
      e = calcula(0, procs_count, 3, procs_info, procs_count, 0);
      if(e != -4)
      {
        return e;
      }
    }
    if(prioridad_actual == 2){
      e = calcula(0, procs_count, 1, procs_info, procs_count, 0);
      if(e != -4)
      {
        return e;
      }
      e = calcula(indice, procs_count, 2, procs_info, procs_count, 0);
      if(e != -4)
      {
        return e;
      }
      e = calcula(0, indice, 2, procs_info, procs_count, 0);
      if(e != -4)
      {
        return e;
      }
      e = calcula(0, procs_count, 3, procs_info, procs_count, 0);
      if(e != -4)
      {
        return e;
      }
    }
    e = calcula(0, procs_count, 1, procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, procs_count, 2, procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
    e = calcula(indice, procs_count, 3, procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, indice, 3, procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
    return -1;
}
//En este mlfq el tiempo q cada proceso esta con una prioridad es de 80ms
//y el slice time del rr es de 40ms
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid)
{
  int e;
  for(e = 0; e < procs_count;e++)
  {
    if(prioridad[procs_info[e].pid] == -1)
    {
      prioridad[procs_info[e].pid] = 1;
    }
  }
  if(curr_pid == -1)
  {
    return proceso_terminado(procs_info, procs_count);
  }
  tiempo[curr_pid] += 10;//actualiza el tiempo del proceso en esa prioridad
  s = s + 10;//actualiza el tiempo q falta para poner todos los procesos en la 
            //en la maxima prioridad
  //Si se llega a 300 se ponen todos los procesos en la maxima prioridad
  if(s == 300)
  {
    for(e = 0; e < procs_count;e++)
    {
      if(prioridad[procs_info[e].pid] != -1 && prioridad[procs_info[e].pid] != 1)
      {
        prioridad[procs_info[e].pid] = 1;
        tiempo[procs_info[e].pid] = 0;
      }
    }
  }
  int c;
  c = procs_info[indice].on_io;
  if(tiempo[procs_info[indice].pid] != 40 && tiempo[procs_info[indice].pid] != 80 && curr_pid != -1 && prioridad[procs_info[indice].pid] == 1 && procs_info[indice].on_io == 0)
  {
    return curr_pid;
  }
  //busca el proceso de mayor prioridad a devolver
  if(prioridad[procs_info[indice].pid] == 1)
  {   
    if(tiempo[procs_info[indice].pid] == 80){
    tiempo[procs_info[indice].pid] = 0;
    prioridad[procs_info[indice].pid]++;}
    e = calcula(indice+1, procs_count, 1,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, indice+1, 1,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, procs_count, 2,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, procs_count, 3,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
  }
  if(prioridad[procs_info[indice].pid] == 2)
  {
    e = calcula(0, procs_count, 1,procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
    if(tiempo[procs_info[indice].pid] != 40 && tiempo[procs_info[indice].pid] != 80 && procs_info[indice].on_io != 1)
    {
      return curr_pid;
    }
    if(tiempo[procs_info[indice].pid] == 80)
    {
      prioridad[procs_info[indice].pid]++;
      tiempo[procs_info[indice].pid] = 0;
    }
    e = calcula(indice+1, procs_count, 2,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, indice, 2,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
    e = calcula(0, procs_count, 3,procs_info, procs_count, c);
    if(e != -4)
    {
      return e;
    }
  }
  e = calcula(0, procs_count, 1,procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
  e = calcula(0, procs_count, 2,procs_info, procs_count, 0);
    if(e != -4)
    {
      return e;
    }
  if(tiempo[procs_info[indice].pid] != 40 && procs_info[indice].pid == 0 && procs_info[indice].on_io == 0)
  {
    return curr_pid;
  }
  tiempo[procs_info[indice].pid] = 0;
  indice++;
  if(indice >= procs_count)
  {
    indice = 0;
  }
  if(procs_info[indice].on_io == 1)
  {
    for(e = indice; e < procs_count; e++)
    {
      if(procs_info[e].on_io == 0)
      {
        return procs_info[e].pid;
      }
    }
  }
  return procs_info[indice].pid;
}

/*int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time =
      procs_info[0].executed_time;  // Tiempo que lleva el proceso activo
                                    // (curr_time - arrival_time)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

  return -1;
}*/

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  if (strcmp(name, "rr") == 0) return *rr_scheduler;

  if (strcmp(name, "mlfq") == 0)
  {
    actualizar();
    return *mlfq_scheduler;
  } 
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
