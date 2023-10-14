#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h> // para utilizar el tipo bool

#include "simulation.h"

// Variables globales para la estrategia Round Robin
const SLICE_TIME = 50;
// guarda el tiempo de la simulación actual cuando se selecciona un proceso distinto del proceso con PID curr_pid
int start_time = 0;
// puntero que indica el proceso de la lista procs_info 
int proc_index = 0; 


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
    // Verificaciones 
  //  printf("Cantidad de procesos activos: %d \n",procs_count);
   // valor del pid del proceso actual cuando ha finalizado
  //  printf("PID del proceso actual: %d\n", curr_pid);
   // si el proceso actual ha finalizado, su pid no se encuentra en la lista de procesos activos
  //  for( int i = 0; i < procs_count; i++) {
  //   if(procs_info[i].pid == curr_pid) {
  //     printf("El proceso esta en la lista \n");
  //     printf("Tiempo de ejecucion del proceso actual: %d \n",procs_info[i].executed_time);  
  //   }
  //  }  
   
  //  printf("NEXT PID: %d \n",procs_info[0].pid);
  //  printf("Tiempo de la simulacion: %d \n",curr_time);
  
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // Devuelve el pid del proceso actual hasta que termine su tiempo de ejecución
  if(curr_pid != -1) {
    return curr_pid;
  }
  // PID del proceso de menor tiempo de duración
  int shortest_job_pid = procs_info[0].pid; 
  
  for(int j = 1; j < procs_count; j++) {
    // duración del proceso j-ésimo de la lista de procesos activos  
     int duration = process_total_time(procs_info[j].pid);
    // Si el proceso j-ésimo tiene un tiempo de duración menor que el proceso de menor duración encontrado hasta ahora
    // actualiza el PID del proceso de menor duración 
     if(duration < process_total_time(shortest_job_pid)) {       
       shortest_job_pid = procs_info[j].pid;
     }    
  }   
  // Se devuelve el PID del proceso de menor duración entre todos los procesos activos 
  return shortest_job_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
    // PID del primer proceso activo en la lista de procesos activos
    int next_pid = procs_info[0].pid;
    // Tiempo restante para la ejecución del primer proceso de la lista
    int shortest_time_to_complete =  process_total_time(next_pid) - procs_info[0].executed_time;
    
    // Buscar el proceso activo que menor tiempo le quede para completar su ejecución
    for(int i = 1; i < procs_count; i++){
      // Tiempo restante para la ejecución del proceso i-ésimo 
      int time_to_complete_job = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      // Si el tiempo restante para la ejecución del proceso i-ésimo es menor que el valor de la variable
      // shortest_time to complete; actualizar el valor de next_pid y shortest_time_to_complete
      if(time_to_complete_job < shortest_time_to_complete) {
        next_pid = procs_info[i].pid;
        shortest_time_to_complete = time_to_complete_job;
      }
    }
  // Devuelve el PID del proceso al que le queda menos tiempo de ejecución
  return next_pid;
}

int round_robin_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  // estado inicial de la simulación donde no se ha ejecutado ningún proceso
  if(curr_pid < 0 && start_time == 0) {    
    start_time = curr_time;
    return procs_info[proc_index].pid;
  }
  // valor true si se ha consumido un tiempo igual a SLICE_TIME desde que se escogió el proceso actual
  bool change = (SLICE_TIME + start_time) == curr_time;
  
  // si se ha consumido un tiempo igual a SLICE_TIME y el próximo valor del puntero coincide con procs_count
  if(change && proc_index + 1 == procs_count) {
    // Actualizar variables globales
    proc_index = 0;
    start_time = curr_time;

    return procs_info[proc_index].pid;
  }
  if(change){
    // Actualizar variables globales
    proc_index = proc_index + 1;
    start_time = curr_time;

    return procs_info[proc_index].pid;
  }
  // si la cantidad de procesos activos coincide con el valor del puntero
  if(proc_index == procs_count){
    proc_index = 0;
  }
  // si el proceso actual ha terminado, 
  // cambiar de proceso sin actualizar el valor del puntero
  if(curr_pid < 0) {
    return procs_info[proc_index].pid;
  }
  
  // devuelve el PID del proceso actual
  return curr_pid;
}
int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
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

  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  
  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
