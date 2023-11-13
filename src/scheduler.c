#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

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


int Comp(pair_t A,pair_t B){
  if(A.fs<B.fs){
    return 1;
  }
  if(A.fs>B.fs)return 0;
  return A.sc<=B.sc;
  
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(curr_pid!=-1)return curr_pid;
  //tupla <tiempo para completar, indice en el array de Procesos>
  pair_t Ans;
  Ans.fs=1e9;
  Ans.sc=-1;
  for(int i=0;i<procs_count;i++){
    pair_t Cur;
    Cur.fs=process_total_time(procs_info[i].pid);
    Cur.sc=i;
    if(Comp(Cur,Ans)){
      Ans=Cur;
    }
  }
  if(Ans.sc==-1){
    return -1;
  }
  return procs_info[Ans.sc].pid; 

}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //tupla <tiempo para completar, indice en el array de Procesos>
  pair_t Ans;
  Ans.fs=1e9;
  Ans.sc=-1;
  for(int i=0;i<procs_count;i++){
    pair_t Cur;
    Cur.fs=process_total_time(procs_info[i].pid)-procs_info[i].executed_time;
    Cur.sc=i;
    if(Comp(Cur,Ans)){
      Ans=Cur;
    }
  }
  if(Ans.sc==-1){
    return -1;
  }
  return procs_info[Ans.sc].pid; 

}

void Q_push(queue_t *Q,int val){
     assert(Q->Tail<500);
     Q->Arr[Q->Tail]=val;
     Q->Tail++;
     Q->Sz++;
     if(Q->Tail==500){
      int NewAr[500];
      for(int i=0;i<Q->Sz;i++){
        NewAr[i]=Q->Arr[Q->Head+i];
      }
      memset(Q->Arr,0,sizeof(Q->Arr));
      Q->Tail=Q->Sz;
      Q->Head=0;
      for(int i=0;i<Q->Sz;i++){
         Q->Arr[i]=NewAr[i];
      }
     }
}
int Q_front(queue_t *Q){
  return Q->Arr[Q->Head];
}

void Q_pop(queue_t *Q){
    Q->Head++;
    Q->Sz--;
}
int Flag[500000];
int Old[500000];
queue_t Qrr;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  
  for(int i=0;i<procs_count;i++){
      if(!Old[procs_info[i].pid]){
        Q_push(&Qrr,procs_info[i].pid);
        Old[procs_info[i].pid]=1;
      }
      Flag[procs_info[i].pid]=1;
  }
 while(Qrr.Sz && !Flag[Q_front(&Qrr)]){
      Q_pop(&Qrr);
 }
 int ans=-1;
  if(curr_time%30==0){
    int pid=Q_front(&Qrr);
    Q_pop(&Qrr);
    Q_push(&Qrr,pid);
    while(Qrr.Sz && !Flag[Q_front(&Qrr)]){
      Q_pop(&Qrr);
    }
    ans=Q_front(&Qrr);
  }else{
    ans=Q_front(&Qrr);
  }
  for(int i=0;i<procs_count;i++){
      Flag[procs_info[i].pid]=0;
  }

  return ans;
}

int IO[500000];
int ExecutedT[500000];
queue_t MLQ[4];

void PriorityBoost(){
    for(int i=1;i<4;i++){
       while(MLQ[i].Sz){
          int pid=Q_front(&MLQ[i]);
          Q_pop(&MLQ[i]);
          Q_push(&MLQ[0],pid);  
       } 
    }
}
int min(int a,int b){
  if(a<b)return a; 
  return b; 
}


int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {

    for(int i=0;i<procs_count;i++){
      if(!Old[procs_info[i].pid]){
        Q_push(&MLQ[0],procs_info[i].pid);
        Old[procs_info[i].pid]=1;
      }
      if(procs_info[i].on_io){
        IO[procs_info[i].pid]=1;
      }
      ExecutedT[procs_info[i].pid]=procs_info[i].executed_time;
      Flag[procs_info[i].pid]=1;
    }
    int ans=-1;
   for(int i=0;i<4;i++){
    int Flg=1;
    int ini=-1;
      while(MLQ[i].Sz){
        if(!Flag[Q_front(&MLQ[i])]){
          Q_pop(&MLQ[i]);
          Old[Q_front(&MLQ[i])]=0;
          continue;
        }
        if(Q_front(&MLQ[i])==curr_pid && ExecutedT[Q_front(&MLQ[i])]%40==0 && Flg){
          int pid=Q_front(&MLQ[i]);
          Q_pop(&MLQ[i]);
          Q_push(&MLQ[min(i+1,3)],pid);
          Flg=0;
          continue;
        }
        if(IO[Q_front(&MLQ[i])]){
          int pid=Q_front(&MLQ[i]);
          if(ini==pid){
            break;
          }
          if(ini==-1){
            ini=pid;
          }        
          Q_pop(&MLQ[i]);
          Q_push(&MLQ[i],pid);
          continue;
        }
        break;
      }
      if(MLQ[i].Sz==0){
        continue;
      }
      if(IO[Q_front(&MLQ[i])])continue;
      ans=Q_front(&MLQ[i]);
      break;
    }

  if(curr_time%200==0){
    PriorityBoost();
  }
  for(int i=0;i<procs_count;i++){
      Flag[procs_info[i].pid]=0;
      IO[procs_info[i].pid]=0;
  }
  return ans;
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
