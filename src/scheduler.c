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

int *timePB;

int last_process;

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  last_process = curr_pid;
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

 int current = 0;
 int finished = 0;

if(procs_count == 0)return -1;

if(curr_pid != -1){
  printf("mayor");
  for(int c = 0 ;c < procs_count;c++){ //encontrar el proceso actual
    if(procs_info[c].pid == curr_pid ){
      current = c ;
    break;
  }
  }
  last_process = current;

}else{
  printf("menor");
  if(last_process == 0){
  //si el anterior era el primer proceso entonces buscar el primero que este disponible
  for(int c = 0;c < procs_count;c++ ){

    if(procs_info[c].on_io == 0 && process_total_time(procs_info[c].pid) - procs_info[c].executed_time >= 0){
       
      last_process = c;
      return procs_info[c].pid;
      
    }

  }

  }else{

  return procs_info[last_process].pid;
  }
}


if(curr_time % 50 == 0 || procs_info[current].on_io == 1 || process_total_time(procs_info[current].pid) - procs_info[current].executed_time < 0 || curr_pid < 0){
printf("time");
 int found = 0;//variable que determina si se encontro un proceso que no esta en IO a la derecha

  if(current + 1 < procs_count){
  for(int c = current + 1 ;c < procs_count;c = c + 1){// si existe algun proceso delante del actual que no esta en IO este bucle lo va a encontrar
    
    if(procs_info[c].on_io == 0 && process_total_time(procs_info[c].pid) - procs_info[c].executed_time >= 0){ 
      current = c;
      found = 1;
      break; 
    }
  }

  }

  if(found == 0){//no se encontro a la derecha o llego al final
    for(int a = 0 ;a < current ;a = a + 1){// si existe algun proceso detras del actual que no esta en IO este bucle lo va a encontrar
           
      if((procs_info[a].on_io == 0) && (process_total_time(procs_info[a].pid) - procs_info[a].executed_time >= 0) ){ 
            
        current = a;
        break; 
      }
    }
  }

 return procs_info[current].pid;

}else{

return curr_pid;

}

}//fin de rr

void restart(){
 for(int c = 0 ;c < 200;c++){

 timePB[c] = 0;

 }

}

int priority_rr(proc_info_t *procs_info, int procs_count,int curr_pid,int current_process_level){

 

  int current = 0;

  for(int c = 0 ;c < procs_count;c++){ //encontrar el proceso actual
    if(procs_info[c].pid == curr_pid ){
      current = c ;
    break;
  }
  }

 int found = 0;//variable que determina si se encontro un proceso que no esta en IO a la derecha y tenga el nivel correcto

  
  for(int c = current + 1 ;c < procs_count;c++){// si existe algun proceso delante del actual que no esta en IO este bucle lo va a encontrar
 
    if(procs_info[c].on_io == 0 && ( timePB[procs_info[c].pid] / 50 == current_process_level ) ){ 
      current = c;
      found = 1;
      break; 
    }
  }

  if(found == 0){//no se encontro a la derecha
    for(int c = 0 ;c < current;c++){// si existe algun proceso detras del actual que no esta en IO este bucle lo va a encontrar
 
      if(procs_info[c].on_io == 0 && ( timePB[procs_info[c].pid] / 50 == current_process_level ) ){ 
        current = c;
        break; 
      }
    }
  }


 return procs_info[current].pid;


 

}

 


int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

 int current = curr_pid;
 int current_process_position;

 if(curr_time % 1000 == 0){// priority boost, reinicia los tiempos a cero cada 200ms y ejecuta el proximo proceso que no esta en IO
  
  restart();

  current = rr_scheduler(procs_info,procs_count,curr_time,curr_pid);

 }else{
   
    for(int c = 0 ;c < procs_count;c++){ //encontrar la posicion del proceso actual
     if(procs_info[c].pid == curr_pid ){
      current_process_position = c ;
     }
    }
  
  int current_process_level = timePB[curr_pid] / 50;//nivel del proceso actual

  for(int c = 0;c < 200;c++){

    //aqui encuentra el primero de los procesos con mayor prioridad que no estan en IO
    if((timePB[procs_info[c].pid] / 50 < timePB[current] / 50) && (procs_info[c].on_io == 0))current = procs_info[c].pid;

  }

  if( timePB[current] / 50 == current_process_level ){
  /*si el proceso con mayor prioridad tiene igual prioridad al proceso actual entonces usar round robin desde el proceso actual
  */
    return priority_rr(procs_info,procs_count,curr_pid,current_process_level);

  }



 }

    for(int c = 0 ;c < procs_count;c++){ 
     if(procs_info[c].pid == current ){
      current_process_position = c ;
     }
    }

 if(procs_info[current_process_position].on_io == 0){
  timePB[current] = timePB[current] + 10;//aumenta en 10 su tiempo en ejecucion desde el priority boost si no esta en IO
  
 }
 
 
 return current;

}//mlfq



// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name,int *times[200]) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.
  
  timePB = times;

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
