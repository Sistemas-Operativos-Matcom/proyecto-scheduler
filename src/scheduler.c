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
  

  return -1;
}

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
  // if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
