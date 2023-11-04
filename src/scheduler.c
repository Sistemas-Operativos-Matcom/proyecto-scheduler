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



int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
    // Se devuelve el PID del primer proceso de todos los disponibles
    return procs_info[0].pid;
}


int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
    // Se devuelve el PID del proceso con menor tamaño
    if(curr_pid!=-1)return curr_pid;
    int id=0;
    int PID=-1;
    for(int i=0;i<procs_count;i++){ // Si encontramos uno mejor nos lo quedamos
      if(process_total_time(procs_info[i].pid)<=process_total_time(procs_info[id].pid)){
        id=i;
        PID=procs_info[i].pid;
      }
    } 
    return PID;
} 

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
    // Se devuelve el PID con el menor tiempo a ser completado.
    if(curr_pid!=-1)return curr_pid;
    int PID=-1;
    int best=process_total_time(procs_info[0].pid)-procs_info[0].executed_time;
    
    for(int i=0;i<procs_count;i++){
      if(process_total_time(procs_info[i].pid)-procs_info[i].executed_time <= best){ //Si encontramos uno mejor no los quedamos
        PID=procs_info[i].pid;
        best=process_total_time(procs_info[i].pid)-procs_info[i].executed_time;
      }
    } 
    return PID;
} 

#define quantum 5
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    // Round Robin, en todo su esplendor 
    if(curr_pid==-1)return procs_info[0].pid;
    
    for(int i=0;i<procs_count;i++){
      if(procs_info[i].pid==curr_pid){
          if(curr_time%quantum!=0){ // SI aun no toca cambiar de proceso nos quedamos con el que estamos
            return curr_pid;
          }
          return procs_info[ (i+1) %procs_count].pid; //Sino pasamos al siguiente
      }
    }

    return -1;
}



int max_time_per_queue = 50;
int boost_time = 100;
int current_queue[1000000];
int on_queue_time[1000000];

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid){
    // Multi-level Feedback Queue 

    for (int i=0;i<procs_count;i++){
      int pid = procs_info[i].pid;
      if(!procs_info[i].on_io){ // Si no esta en IO
        on_queue_time[pid] += 10;
      }
      if (on_queue_time[pid] >= max_time_per_queue){ // Ha consumido todo su tiempo en esta cola
        current_queue[pid]++;
        on_queue_time[pid] = 0;
      }
    }

    if(curr_time % boost_time == 0){ // Si toca boostear mandamos a todos para la misma cola
      for(int i=0; i<procs_count; i++){
        int pid = procs_info[i].pid;
        current_queue[pid] = 0;
        on_queue_time[pid] = 0;
      }
    }

    int cnt_not_on_io=0; // Contamos cuantos no estan en IO
    for(int i=0;i<procs_count;i++){
      if( !procs_info[i].on_io ){ 
        cnt_not_on_io++;
      }
    }
    if(cnt_not_on_io==0)return -1;
    
    
    proc_info_t not_on_io[cnt_not_on_io]; // Guardamos en un arreglo los que no estan en IO
    int wr=0;
    for(int i=0;i<procs_count;i++){
      if(!procs_info[i].on_io){ 
        not_on_io[wr]=procs_info[i];
        wr++;
      }
    }
    
    int minp=1e9;
    int prior_queue_size=0;
    for(int i=0;i<cnt_not_on_io;i++){ // De los que no estan en IO tomamos guardamos los que tengan mayor prioridad y contamos cuantos son
      int pid=not_on_io[i].pid;
      if(current_queue[pid]<minp){
        minp=current_queue[pid];
        prior_queue_size = 0;
      }
      if(current_queue[pid]==minp){
        prior_queue_size++;
      }
    }

    proc_info_t priority_procs[prior_queue_size]; // Guardamos en un arreglo los elementios de cola con mayor prioridad
    wr=0;
    for (int i=0;i<cnt_not_on_io;i++){
      int pid=not_on_io[i].pid;
      if (current_queue[pid]==minp){
        priority_procs[wr] = not_on_io[i];
        wr++;
      }
    }
    return rr_scheduler(priority_procs, prior_queue_size, curr_time, curr_pid); 
}




schedule_action_t get_scheduler(const char *name) {

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
