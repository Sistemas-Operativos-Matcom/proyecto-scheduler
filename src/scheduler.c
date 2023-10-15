#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
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

int currentIndexRR = 0;
int amountTimerInterrupts = 0;
int last_procs_count = 0;

//punteros para emular una lista

int last_of_q0 = 0; 
int last_of_q1 = 0;
int last_of_q2 = 0;

//listas de prioridades mayor 0 menor 2

int q[MAX_PROCESS_COUNT][3];

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

//testear

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  int minTime = INT_MAX;
  int index = 0;

  for(int i = 0; i < procs_count; i++){ //buscar el proceso con menor tiempo de ejecucion total
    int _processTotalTime = process_total_time(procs_info[i].pid);
    if(_processTotalTime < minTime){
      index = i;
      minTime = _processTotalTime;
    }
  }                  

  return procs_info[index].pid;
}

//testear
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  int minTime = INT_MAX;
  int index = 0;

  for(int i = 0; i < procs_count; i++){ // buscar el proceso con menor tiempo restante (total - transcurrido)
    int _processTotalTime = process_total_time(procs_info[i].pid);
    if(_processTotalTime-procs_info[i].executed_time < minTime){
      index = i;
      minTime = _processTotalTime-procs_info[i].executed_time; 
    }
  }                  

  return procs_info[index].pid;
}

//testear
//como averiguo cuanto le falta por terminar al proceso curr_pid

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                  int curr_pid, int time_slice) { //time_slice sería cuantos timer-interupt son necesarios para cambiar de proceso
  time_slice = 5;

  if(amountTimerInterrupts < time_slice - 1 && curr_pid != -1){ // si aun no se cumple el time slice seguimos con el mismo proceso
    amountTimerInterrupts++;
    return curr_pid;
  }
  else{ // toca cambiar de proceso
    int _pid;
   
     // aqui se contempla la posibilidad de que el proceso termino en el time slice pasado entonces no es necessario 
     // actualizar el index solo si el proceso se mantiene en su posicion en el array
    if(procs_info[currentIndexRR].pid == curr_pid && curr_pid != -1){
      currentIndexRR++;
    }
    
    if(currentIndexRR >= procs_count){ // cuando ejecutamos el ultimo en la lista le toca al del incio otra vez
      currentIndexRR = 0;
    }
    
    _pid = procs_info[currentIndexRR].pid;
    amountTimerInterrupts = 0;

    return _pid;
  }
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid, int time_slice, int priority_boost) {
  time_slice = 5;
  priority_boost = 100;
  
  // agregar proceso nuevo a q0
  if(procs_count > last_procs_count){
    q[last_of_q0++][0] = procs_info[procs_count-1].pid;
  }

  last_procs_count = procs_count;

}

/*int remove_from_array(int *array, int pid){
  for
}*/

int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)

  // Información que puedes obtener de un proceso
  int pid = procs_info[0].pid;      // PID del proceso
  int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
                                    // realizando una opreación IO
  int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
                                                // ejecutado (en CPU o en I/O)

  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  int duration = process_total_time(pid);

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
  if (strcmp(name, "rr") == 0) return *round_robin_scheduler;
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
