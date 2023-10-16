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
  if (curr_pid != -1) return curr_pid;
  int next_proc = procs_info[0].pid;
  int next_proc_time = process_total_time(procs_info[0].pid);
  for (int i = 0; i < procs_count; i++)
  {
    if (next_proc_time > process_total_time(procs_info[i].pid)){
      next_proc_time = process_total_time(procs_info[i].pid);
      next_proc = procs_info[i].pid;
    }
  }
  return next_proc;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, 
                  int curr_pid) {
  int next_proc = procs_info[0].pid;
  int next_proc_time = process_total_time(procs_info[0].pid);
  for (int i = 0; i < procs_count; i++)
  {
    if (next_proc_time > process_total_time(procs_info[i].pid)){
      next_proc_time = process_total_time(procs_info[i].pid);
      next_proc = procs_info[i].pid;
    }
  }
  return next_proc;
}

int aux = 0;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, 
                  int curr_pid) {
  int time_slice = 50;
  if (curr_time % time_slice == 0 || curr_pid == -1){
    if (curr_pid == -1) aux--;
    if (aux >= procs_count - 1) aux = 0;
    else aux++;
    return procs_info[aux].pid;
  }
  return curr_pid;
}


//Metodo auxiliar para que todos los procesos
//queden en las primeras posiciones
void clean(int A[]){
  for (int i = 0; i < 1000; i++)
  {
    if (A[i] == -2){
      for (int j = i + 1; j < 1000; j++)
      {
        if (A[j] != -2){
          A[i] = A[j];
          A[j] = -2;
          break;
        }
        if (j = 999) return;
      }
    }
  }
}

//Metodo auxiliar para que todos los procesos que se completaron
//se eliminen de las colas
void cleanInverse(int A[], proc_info_t *procs_info, int procs_count){
  int clean_bool = 0;
  for (int i = 0; i < 1000; i++)
  {
    clean_bool = 0;
    if (A[i] != -2){
      for (int j = 0; j < procs_count; j++)
      {
        if (procs_info[j].pid == A[i]) {
          clean_bool = 1;
          break;
        }
      }
      if (clean_bool == 1) continue;
      A[i] = -2;
    }
  }
}

proc_info_t findProc(proc_info_t *procs_info, int procs_count, int pid){
  for (int i = 0; i < procs_count; i++)
  {
    if (pid == procs_info[i].pid) return procs_info[i];
  }
}

//Creacion de las colas
int queue1[1000];
int queue2[1000];
int queue3[1000];

int boolfill = 0;
void fill(){
  for (int i = 0; i < 1000; i++)
  {
    queue1[i] = -2;
    queue2[i] = -2;
    queue3[i] = -2;
  }
}

int auxM1 = 0;
int auxM2 = 0;
int auxM3 = 0;

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, 
                  int curr_pid) {
  if (boolfill == 0) fill(); boolfill++;

  clean(queue1);
  clean(queue2);
  clean(queue3);

  cleanInverse(queue1, procs_info, procs_count);  
  cleanInverse(queue2, procs_info, procs_count);  
  cleanInverse(queue3, procs_info, procs_count);

  clean(queue1);
  clean(queue2);
  clean(queue3);

  int countQ1 = 0;
  int countQ2 = 0;
  int countQ3 = 0;

  int b1 = 0;
  int b2 = 0;
  int b3 = 0;

  //Comprueba si todos los procesos estan en alguna cola
  //sino lo incluyo en la primera cola

  //recorre el procs_info
  for (int i = 0; i < procs_count; i++)
  {
    //recorre el queue1
    for (int j = 0; j < procs_count; j++)
    {
      if (procs_info[i].pid == queue1[j]) {
        countQ1++; 
        b1 = 1;
        break;
      }
    }
    //recorre el queue2
    for (int j = 0; j < procs_count; j++)
    {
      if (procs_info[i].pid == queue2[j]) {
        countQ2++; 
        b2 = 1;
        break;
      }
    }
    //recorre el queue3
    for (int j = 0; j < procs_count; j++)
    {
      if (procs_info[i].pid == queue3[j]) {
        countQ3++; 
        b3 = 1;
        break;
      }
    }
    if (b1 == 0 && b2 == 0 && b3 == 0){
      queue1[countQ1] = procs_info[i].pid;
      countQ1++;
    }
      b1 = 0;
      b2 = 0;
      b3 = 0;
  }
  
  //priority boost
  int boost = 200;
  if (curr_time % boost == 0)
  {
    for (int i = countQ1; i < countQ1 + countQ2; i++)
    {
      queue1[i] = queue2[i - countQ1];
      queue2[i - countQ1] = -2;
    }
    for (int i = countQ1 + countQ2; i < countQ1 + countQ2 + countQ3; i++)
    {
      queue1[i] = queue3[i - countQ1 - countQ2];
      queue3[i - countQ1 - countQ2] = -2;
    }
    countQ1 = countQ1 + countQ2 + countQ3;
    countQ2 = 0;
    countQ3 = 0;
  }  

  int baja_cola;
  int time_slice = 50;

  //rr en q1
  for (int i = 0; i < countQ1; i++)
  {
    //printf("q1 ");
    if (findProc(procs_info, procs_count, queue1[i]).on_io) continue;
    if (curr_time % time_slice == 0 && curr_time % boost != 0){
      if (auxM1 >= countQ1 - 1) auxM1 = 0;
      //queue1[auxM1] se devuelve y baja de cola 
      baja_cola = queue1[auxM1];
      queue1[auxM1] = -2;
      queue2[countQ2] = baja_cola;
      if (countQ1 == 1) return queue2[0];
      return queue1[auxM1 + 1];
    }
    else if (curr_pid == -1) return queue1[auxM1];
    if (countQ1 == 1) if (findProc(procs_info, procs_count, queue1[0]).on_io) return queue1[0];
    if (findProc(procs_info, procs_count, curr_pid).on_io) break;
    return curr_pid;
  }

  //rr en q2
  for (int i = 0; i < countQ2; i++)
  {
    //printf("q2 ");
    if (findProc(procs_info, procs_count, queue2[i]).on_io) continue;
    if (curr_time % time_slice == 0 && curr_time % boost != 0){
      if (auxM2 >= countQ2 - 1) auxM2 = 0;

      //queue2[auxM2] se devuelve y baja de cola
      baja_cola = queue2[auxM2];
      queue2[auxM2] = -2;
      queue3[countQ3] = baja_cola;
      if (countQ2 == 1) return queue3[0];
      return queue2[auxM2 + 1];
    }
    else if (curr_pid == -1) return queue2[auxM2];
    if (countQ2 == 1) if (findProc(procs_info, procs_count, queue2[0]).on_io) return queue2[0];
    if (findProc(procs_info, procs_count, curr_pid).on_io) break;
    return curr_pid;
  }

  //rr en q3
  for (int i = 0; i < countQ3; i++)
  {
    //printf("q3 ");
    if (findProc(procs_info, procs_count, queue3[i]).on_io) continue;
    if (curr_time % time_slice == 0 && curr_time % boost != 0){
      if (auxM3 >= countQ3 - 1) auxM3 = 0;
      else auxM3++;
      if (countQ3 == 1) return queue3[0];
      return queue3[auxM3];
    }
    else if (curr_pid == -1) return queue3[auxM3];
    if (countQ3 == 1) if (findProc(procs_info, procs_count, queue3[0]).on_io) return queue3[0];
    if (findProc(procs_info, procs_count, curr_pid).on_io) break;
    return curr_pid;
  }
  for (int i = 0; i < procs_count; i++)
  {
    if (findProc(procs_info, procs_count, procs_info[i].pid).on_io) continue;
    return procs_info[i].pid;
  }
  
  return procs_info[0].pid;
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;
  if (strcmp(name, "rr") == 0) return *rr_scheduler;
  if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}






//ideas para io
//hacer que el aarray de int sea un array de int, bool
//