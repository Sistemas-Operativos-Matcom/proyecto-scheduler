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
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) 
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)
  //
  // Información que puedes obtener de un proceso
  // int pid = procs_info[0].pid;      // PID del proceso
  // int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                     // realizando una opreación IO
  // int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)
  //
  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
//   int duration = process_total_time(pid);
  //
//   return -1;
// }

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int sdur = __INT_MAX__;
  int spid = -1;
  
  //Recorre el array para determinar el proceso al que le queda menos.
  for (int i = 0; i < procs_count; i++)
  {
    int cpid = procs_info[i].pid;
    int duration = process_total_time(cpid);
    
    if (duration < sdur)
    {
      sdur = duration;
      spid = cpid;
    }
  }
  return spid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  //Si no ha terminado el proceso, seguir ejecutándolo.
  if (curr_pid != -1)
    return curr_pid;
  
  int sdur = __INT_MAX__;
  int spid = -1;

  for (int i = 0; i < procs_count; i++)
  {
    int cpid = procs_info[i].pid;
    int duration = process_total_time(cpid);
    if (duration < sdur)
    {
      sdur = duration;
      spid = cpid;
    }
  }
  return spid;
}

int rr_index = 0;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int time_slice = 50;
  int last;

  //last es 1 si el proceso es el ultimo proceso; 0 en caso contrario
  if (rr_index >= procs_count-1)
    last = 1;
  else
    last = 0;

  //Tiempo que se ha ejecutado un proceso
  int exec_time = procs_info[rr_index].executed_time;

  //Caso inicial. Como rr_index inicial es 0, si el proceso 0 no se ha ejecutado, comenzar a ejecutarlo.
  if (exec_time == 0)
    return procs_info[rr_index].pid;

  //Si se cumple, no ha llegado al time slice y se sigue ejecutando el mismo proceso
  if (exec_time % time_slice != 0)
  {
    if (curr_pid != -1)
      return curr_pid;
  }

  //En el caso del MLFQ, si el proceso actual se encuentra marcado es porque pertenece a una cola
  //Por tanto, cuando consuma el time slice hay que bajarle la prioridad
  //Esto es: sacarlo de la cola actual y ponerlo en la cola siguente

  

  //Si no hay un proceso siguiente, comienza por el proceso 0.
  //De lo contrario, devuelve el pid del siguiente proceso.
  if (last != 1)
  {
    rr_index++;
    return procs_info[rr_index].pid;   
  }
  rr_index = 0;
  return procs_info[rr_index].pid;
}


// -------------------------  METODOS NECESARIOS PARA IMPLEMENTAR MLFQ---------------------------

//Si un proceso terminó, devuelve 1, Else devuelve 0
//Recibe el pid del proceso, la lista y la cantidad de procesos actuales activos
int process_ended(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i<procs_count; i++)
  {
    if (procs_info[i].pid == pid)
      return 0;
  }
  return 1;
}

//Retorna el proceso que se está ejecutando actualmente
proc_info_t get_current_process(proc_info_t *procs_info, int procs_count, int c_pid)
{
    for (int i = 0; i<procs_count; i++)
    {
      if (procs_info[i].pid == c_pid)
        return procs_info[i];
    }
    proc_info_t error = {-1, -1, -1};
    return error;
}

//--------------------------------    INICIALIZACIONES DE MLFQ -------------------------------------

//Cola 0 menor prioridad, cola 2 mayor prioridad
  proc_info_t q0[200];
  proc_info_t q1[200];
  proc_info_t q2[200];

  proc_info_t *queues[3] = {q0, q1, q2};

  //Define la cola del proceso que se está ejecutando:
  int active_queue = -1;
  
  //array containing added processes pids. Size = 200
  int added[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  //Me indica si se ha ejecutado o no algún proceso con mlfq. 0 es que no
  int mlfq_begin = 0;

// -----------------------------------  -->     MLFQ   <--  ------------------------------------

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  //tomar todos los procesos de la misma queue y hacerles RR.
  //cuando un proceso consuma el time slice, bajarle la prioridad = bajarlo de cola

  //EL PROCESO 0 SE EJECUTA COMPLETO. ARREGLAR ESTO

  int time_slice = 50;
  //Punto de partida para mlfq. Ejecuta siempre el primer proceso de todos.
  if (mlfq_begin == 0)
  {
    mlfq_begin = 1;
    added[0] = 1;
    q2[0] = procs_info[0];
    active_queue = 2;
    return procs_info[0].pid;
  }

  //De los procesos activos, añadir los que no están en alguna cola a la cola de mayor prioridad (la cola 2):
  for (int i = 0; i < procs_count; i++)
  {
    int index = procs_info[i].pid;
    if (added[index] == 0)    //el proceso no está en cola
    {
      added[index] = 1;       //marcarlo como añadido
      q2[index] = procs_info[i];  //añadirlo a la cola 2
    }
  }
  

  //Si hay algun proceso ejecutandose, seguir ejecutandolo:
  if (curr_pid != -1)
  {
    //Obtengo el proceso actual
    proc_info_t curr_process = get_current_process(procs_info, procs_count, curr_pid);
    //Si no ha consumido el time_slice, seguir ejecutándolo
    if (curr_process.executed_time % time_slice != 0)
      return curr_pid;
    else
    {
      //IMPLEMENTAR:
      //si ya consumio el time slice, tengo que pasar al siguiente proceso si en la cola actual hay alguno.
      //Si en la cola actual no hay ninguno, bajo de cola hasta que haya alguno
      //Actualizar el active_queue si se modificó

      
      //Como ya consumio el time slice:
      //Si la cola no es la de menor prioridad, baja la prioridad del proceso.
      if (active_queue != 0)
      {
        //Quitar el proceso de la cola actual:
        proc_info_t empty_process = {0,0,0};
        queues[active_queue][curr_pid] = empty_process;

        //Mover el proceso a la cola de abajo
        queues[active_queue - 1][curr_pid] = curr_process;
      }
      else //IMPLEMENTAR QUE HACER SI LA COLA FUE LA DE MENOR PRIORIDAD
      {

      }


    }
  }
    
  //ESTO ES LO QUE SE VA A HACER SI NO HAY NINGUN PROCESO EJECUTANDOSE ACTUALMENTE:

  //Recorro las colas en orden a ver cual sera la primera con un proceso pendiente:
  for (int i = 2; i < 3; i--)
  {
    for (int j = 0; j < 200; j++)   
    {
      int c_pid = queues[i][j].pid;
      if (c_pid != 0)
      {
        //Si el proceso no se ha terminado, seguir ejecutandolo:
        if (process_ended(procs_info, procs_count, c_pid) == 0)   //O(n)
          return queues[i][j].pid;
      }
    }
  }
  return -1;
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
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
