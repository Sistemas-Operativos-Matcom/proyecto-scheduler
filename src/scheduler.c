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
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}



int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {



int shortest;//proceso mas corto

shortest = 0;

for(int c = 0 ;c < procs_count;c++){//el candidato inicial para el proceso mas corto es el proceso actual
  if(procs_info[c].pid == curr_pid ){
   shortest = c;
   break;
}

}

if((curr_pid != -1 ) && (procs_info[shortest].on_io == 0) ){return procs_info[shortest].pid;}//solo se deja de ejecutar un proceso cuando esta en IO


for(int c = 0 ; c < procs_count;c++){//encontrar el mas corto que no esta en IO

if((procs_info[c].on_io == 0) && (process_total_time(procs_info[c].pid) < process_total_time(procs_info[shortest].pid) ))shortest = c;

}

  return procs_info[shortest].pid;
}//sjf


int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

int shortest;//proceso mas cerca de finalizar

shortest = 0;

for(int c = 0 ;c < procs_count;c++){//el candidato inicial para el proceso mas cerca del final es el proceso actual
  if(procs_info[c].pid == curr_pid ){
   shortest = c;
   break;
}

}



for(int c = 0 ; c < procs_count;c++){//encontrar el mas cerca del final que no esta en IO

if(procs_info[shortest].on_io == 1)shortest = c;//si el proceso actual esta en IO cambiarlo

if((procs_info[c].on_io == 0) && ( process_total_time(procs_info[c].pid) - procs_info[c].executed_time < process_total_time(procs_info[shortest].pid) - procs_info[shortest].executed_time ) )shortest = c;

}

  return procs_info[shortest].pid;
}


int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

if(curr_time % 20 == 0 ){

int current = 0;

for(int c = 0 ;c < procs_count;c++){ //encontrar el proceso siguiente al actual
  if(procs_info[c].pid == curr_pid ){
   current = c + 1;
   break;
  }
}

int found = 0;//variable que determina si se encontro un proceso que no esta en IO a la derecha

if(procs_info[current].on_io == 1){

  for(int c = current ;c < procs_count;c++){// si existe algun proceso delante del actual que no esta en IO este bucle lo va a encontrar
 
    if(procs_info[c].on_io == 0 ){ 
      current = c;
      found = 1;
      break; 
    }
  }

  if(found == 0){//no se encontro a la derecha
    for(int c = 0 ;c < current;c++){// si existe algun proceso detras del actual que no esta en IO este bucle lo va a encontrar
 
      if(procs_info[c].on_io == 0 ){ 
        current = c;
        break; 
      }
    }
  }

}

return procs_info[current].pid;
}else{

return curr_pid;

}

}//fin de rr

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
  if (strcmp(name, "rr") == 0) return *rr_scheduler;


  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
