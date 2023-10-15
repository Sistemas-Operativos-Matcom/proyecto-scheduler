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
  // Se devuelve el PID del proceso con menor duracion 
  
  //Comprueba si el proceso actual termino de ejecutarse
  if(curr_pid != -1)return curr_pid;

  //Busca el proceso con menos tiempo de ejecucion
  int min= process_total_time(procs_info[0].pid);
  int pid_min= procs_info[0].pid;
  for(int i=1;i<procs_count;i++){
    if(process_total_time(procs_info[i].pid)<min){
      min=process_total_time(procs_info[i].pid);
      pid_min=procs_info[i].pid;
    }
      
  }
  return pid_min;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del proceso mas cercano a terminar

  
  int is_curr=0;
  if(procs_info[0].pid == curr_pid) is_curr=1;
  int stcf=process_total_time(procs_info[0].pid) - procs_info[0].executed_time;
  int pid_ret=procs_info[0].pid;
  //buscamos el proceso con menor tiempo para terminar
  //si encontramos un proceso con igual tiempo que otro, mantemos el proceso actual para evitar cambios de contexto 
  for(int i=1;i<procs_count;i++){
    int stcf_i=process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    if(stcf_i <= stcf ){
      if(stcf_i==stcf && procs_info[i].pid == curr_pid){
        is_curr=1;
        stcf=stcf_i;
        pid_ret=procs_info[i].pid;
      }
      else if(stcf_i<stcf){
        stcf=stcf_i;
        pid_ret=procs_info[i].pid;
        if(is_curr==1) is_curr=0;
        if(procs_info[i].pid == curr_pid) is_curr=1;
      }
    }
  }
  return pid_ret;
}

//Variable para usar en el RR que almacena la posicion del array del ultimo proceso ejecutado
static int curr_pos=0;
static int slice_time_rr=0;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {

  //Va alternando los procesos a ejecutar
  //Si el proceso actual termino ejecuta el siguiente
  if(curr_pid == -1){
    slice_time_rr=0;
    if(curr_pos == procs_count)curr_pos=0;
    return procs_info[curr_pos].pid;
  }
  
  if(slice_time_rr==50){

    //Si el proceso actual no ha terminado y no es el ultimo, ejecuta el siguiente
    if(curr_pos+1 < procs_count){
      curr_pos+=1;
      slice_time_rr=0;
      return procs_info[curr_pos].pid;
    }
      
    //Si el proceso actual no ha terminado y es el ultimo, ejecuta el primero
    curr_pos=0;
    slice_time_rr=0;
    return procs_info[0].pid;
  }
  slice_time_rr+=10;
  return curr_pid;
}

//posicion donde se encuentra el ultimo proceso con prioridad 0 ejecutado
static int posicion_p0=0;
//posicion donde se encuentra el ultimo proceso con prioridad 1 ejecutado
static int posicion_p1=-1;
//posicion donde se encuentra el ultimo proceso con prioridad 2 ejecutado
static int posicion_p2=-1;
//prioridad del ultimo proceso ejecutado
static int last_priority=0;
//variable para llevar el conteo del priority boost
static int priority_boost = 0;

//actualiza las posiciones a las que apuntan posicion_p0, posicion_p1 y posicion_p2
void Update_positions(int priority,proc_info_t *procs_info, int procs_count){
  
  int posicion_main=0;
  int posicion_aux1=0;
  int posicion_aux2=0;
  //identifica a paritr de donde se modifico el array y prepara el contexto
  if(priority==0){
    posicion_main= posicion_p0;
    posicion_aux1= posicion_p1;
    posicion_aux2= posicion_p2;
  }
  if(priority==1){
    posicion_main= posicion_p1;
    posicion_aux1= posicion_p2;
    posicion_aux2= posicion_p0;
  }
  if(priority==2){
    posicion_main= posicion_p2;
    posicion_aux1= posicion_p0;
    posicion_aux2= posicion_p1;
  }

  if(posicion_aux1 > posicion_main) posicion_aux1 -=1;
  if(posicion_aux2> posicion_main) posicion_aux2-=1;

  //encuentra la nueva posicion para prioridad analizada
  int reset=1;
  int found=0;
    for(int i=procs_count-1;i>=0;i--){
      if(*procs_info[i].priority == priority && i>= posicion_main ){
       found=1;
       posicion_main=i;
       reset=0;
      }
      if(*procs_info[i].priority == priority && i < posicion_main && reset==1){
        found=1;
        posicion_main=i;
      }
    }
  if(found==0) posicion_main=-1;

  //actualiza las variables con los valores nuevos
  if(priority==0){
    posicion_p0=posicion_main;
    posicion_p1=posicion_aux1;
    posicion_p2=posicion_aux2;
  }
  if(priority==1){
    posicion_p1=posicion_main;
    posicion_p2=posicion_aux1;
    posicion_p0=posicion_aux2;
  }
  if(priority==2){
    posicion_p2=posicion_main;
    posicion_p0=posicion_aux1;
    posicion_p1=posicion_aux2;
  }

}

//identifica que proceso de una prioridad dada debe ejecutarse
int Find_proc(int priority,int position, proc_info_t *procs_info, int procs_count){
   
  int reset=1;
  int pid_found=-1;
  int pos_found=-1;
   //busca el proceso a ejecutar a partir de la posicion del ultimo proceso de esa prioridad ejecutado
  for(int i=procs_count-1;i>=0;i--){
    if(*procs_info[i].priority == priority && i>= position && procs_info[i].on_io==0  ){     
      pid_found=procs_info[i].pid;
      pos_found=i;
      reset=0;
    }
    if(*procs_info[i].priority == priority && i < position && reset==1 && procs_info[i].on_io==0){    
      pos_found=i;
      pid_found=procs_info[i].pid;
    }
  }
  //guarda la posicion del proceso a ejecutar
  curr_pos = pos_found;
  return pid_found;
}

//identifica el proximo proceso a ejecutar por el mlfq
int Next_proc(int p_scope, proc_info_t *procs_info, int procs_count,int time_slice,int current_pid){
  int n= p_scope;
  int priority_to_find=posicion_p0;
  //busca cual elemento se debe ejecutar en la prioridad mas importante
  //en caso de no encontrar en elemento valido en la prioridad actual, busca en la prioridad siguiente
  while(n<3){
    if(n==1) priority_to_find= posicion_p1;
    if(n==2) priority_to_find= posicion_p2;
    int pid_to_execute = Find_proc(n,priority_to_find,procs_info,procs_count);
    if(pid_to_execute != -1) {
      
      if(n==0) posicion_p0=curr_pos;
      if(n==1) posicion_p1=curr_pos;
      if(n==2) posicion_p2=curr_pos;
      last_priority = n;
      priority_boost+=10;
      if(slice_time_rr !=50){
        slice_time_rr+=10;
        if(time_slice==0 && current_pid != pid_to_execute) slice_time_rr=0; 
      }
      else{
        slice_time_rr=0;
      }
      
      *procs_info[curr_pos].time_in_cpu+=10;
      return pid_to_execute;
    }
    n++;

  }
  
  return -1;
      
}


int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  
  
  //variable q indica la prioridad del proceso mas importante
  int p_scope=3;
  //revisa si ha llegado al tiempo del priority boost
  if(priority_boost==500){
    priority_boost=0;
    posicion_p1=-1;
    posicion_p2=-1;
    //de haber llegado al priority boost, cambia la prioridad de los procesos
    for(int i=0; i<procs_count;i++){
      *procs_info[i].priority=0;
      *procs_info[i].time_in_cpu=0;
    }
    p_scope=0;
    last_priority=0;
  }
  else{
    //calcula la prioridad del proceso mas importante
   for(int i=0; i<procs_count;i++){
    if(*procs_info[i].priority < p_scope) {
      if(*procs_info[i].priority==0 && posicion_p0==-1) posicion_p0=i;
      p_scope=*procs_info[i].priority;
    }

   }
  }
  
  //si el proceso actual termino, actualiza ls variables de las prioridades y ejecuta el siguiente
  if(curr_pid == -1){
    
    Update_positions(last_priority, procs_info, procs_count);
    return Next_proc(p_scope, procs_info, procs_count,0,curr_pid);
  }
  
  //si el proceso actual no termino buscalo en el array
  for(int i=0; i<procs_count;i++){
    if(procs_info[i].pid==curr_pid){
      //si ha consumido su tiempo en cpu bajale la prioridad,actualiza la variable de su prioridad y ejecuta el siguiente proceso
      if(*procs_info[i].time_in_cpu ==60){
        *procs_info[i].time_in_cpu=0;

        int reset=1;
        int pos=-1;
        for(int j=procs_count-1; j>=0;j--){
          if(*procs_info[j].priority == *procs_info[i].priority && j>i){
            reset=0;
            pos=j;
          }
          if(*procs_info[j].priority == *procs_info[i].priority && reset ==1 && j<i ){
            pos=j;
          }
        }
        if(*procs_info[i].priority==0){
          posicion_p0=pos;
          if(posicion_p1==-1) posicion_p1=i;
        } 
        if(*procs_info[i].priority==1){
          posicion_p1=pos;
          if(posicion_p2==-1) posicion_p2=i;
        } 
        if(*procs_info[i].priority != 2) *procs_info[i].priority= *procs_info[i].priority + 1;
        return Next_proc(p_scope,procs_info,procs_count,0,curr_pid);
      }
      //si no ha consumido su tiempo en cpu 
      else{
        //si el proceso esta en I/O devuelve el siguiente proceso a ejecutar
        if(procs_info[i].on_io==1) return Next_proc(p_scope,procs_info,procs_count,0,curr_pid);
        //si no lo esta busca la posicion del siguiente elemento de su misma prioridad
        int reset=1;
        int pos=-1;
        for(int j=procs_count-1; j>=0;j--){
          if(*procs_info[j].priority == *procs_info[i].priority && j>i){
            reset=0;
            pos=j;
          }
          if(*procs_info[j].priority == *procs_info[i].priority && reset ==1 &&  j<i ){
            pos=j;
          }
        }
        //si consumio su tiempo para rr ejecuta el siguiente proceso
        if(slice_time_rr==30){
          //si existe un elemento de su prioridad distinto de el proceso actual, actualiza la posicion referente a esa prioridad
          if(pos!=-1){
            if(*procs_info[i].priority==0) posicion_p0=pos;
            if(*procs_info[i].priority==1) posicion_p1=pos;
            if(*procs_info[i].priority==2) posicion_p2=pos;
          }
          return Next_proc(p_scope,procs_info,procs_count,0,curr_pid);
        }
        //si no ha consumido su tiempo de rr busca el proceso a ejecutar y devuelvelo
        else{
          return Next_proc(p_scope,procs_info,procs_count,1,curr_pid);
        }
 
      }
    }

  }

return 0;

}

/*
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
*/

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

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
