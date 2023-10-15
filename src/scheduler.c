#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "simulation.h"
#define TIME_SLICE 50
#define PRIORITY_BOOST 500
struct Dupla
{
  int pid;
  int time_remaining;
};

/*Round Robin*/
int position = 0;
int proc_init_time;
/*MLFQ*/
int lastarrival = 0;
int proc_init_time;

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
int SJF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  int minimal_time = INT_MAX;
  int return_pid = curr_pid;

  if (curr_pid == -1)
      for (int i = 0; i < procs_count; i++)
      {
        if(minimal_time>process_total_time(procs_info[i].pid))
        {
          minimal_time = process_total_time(procs_info[i].pid);
          return_pid = procs_info[i].pid;
        }
      }
  return return_pid;
}

int STCF_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  int remaining_time = INT_MAX;
  int return_pid = curr_pid;

  for (int i = 0; i < procs_count; i++)
  {
    if(remaining_time>process_total_time(procs_info[i].pid)-procs_info[i].executed_time)
    {
      remaining_time = process_total_time(procs_info[i].pid)-procs_info[i].executed_time;
      return_pid = procs_info[i].pid;
    }
  }
  return return_pid;
}

int Round_Robin(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  if (curr_pid == -1)
    proc_init_time = curr_time;  
  else  if(curr_time - proc_init_time >= TIME_SLICE){
      position++;
      proc_init_time = curr_time;
  }
  if (position >= procs_count)
        position=0;

  return procs_info[position].pid; 
}




//variables para la cola de prioridad alta
struct Dupla hp_queue[MAX_PROCESS_COUNT];//cola
//  tiempo inicio, posicion y cantidad de procesos de esta cola
int hp_proc_init_time, hp_position = 0, hp_procs_count=0;

//variables para la cola de prioridad media
struct Dupla mp_queue[MAX_PROCESS_COUNT];
int mp_proc_init_time, mp_position = 0, mp_procs_count=0;

//variables para la cola de prioridad baja
struct Dupla lp_queue[MAX_PROCESS_COUNT];
int lp_proc_init_time, lp_position = 0, lp_procs_count=0;

//devuelve la posicion de un proceso en la cola, si no esta devuelve -1
int pos_pid(proc_info_t *procs_info, int procs_count,int pid)
{
  int pos = -1;
  for (int i = 0; i < procs_count;i++){
    if (procs_info[i].pid == pid){
        pos = i;
        break;
    }
     
  }
  return pos;  
}
//Devuelve la posición de un proceso en la cola.si no se encuentra devuelve -1
int pos_queue(struct Dupla *queue, int procs_count,int pid)
{
  int pos = -1;
  for (int i = 0; i < procs_count;i++){
    if (queue[i].pid == pid){
        pos = i;
        break;
    }
     
  }
  return pos;  
}

// elimina un proceso de la cola
void del_pid(struct Dupla *procs_info, int procs_count,int pos_pid)
{
  for (int i = pos_pid; i < procs_count-1;i++)
    procs_info[i] = procs_info[i+1];
}

int rr_universal1(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid,
                     int *proc_init_time, int *position) {
  int temp = 0;
  //printf("%d\n",curr_pid);
  if (curr_pid == -1){
      //printf("*************");
      *proc_init_time = curr_time;
      do {
        if (!procs_info[*position].on_io)
            return procs_info[*position].pid;
        else
            *position = *position +1;
        if (*position >= procs_count)
          *position=0;
        if (temp >= procs_count){
            *proc_init_time = -1;
            return -1;
        }
        temp++;
       
      } while(1);
     
  }

  else  if(curr_time - *proc_init_time >= TIME_SLICE){
      *position = *position +1;
      *proc_init_time = curr_time;
      do {
        if (!procs_info[*position].on_io)
            return procs_info[*position].pid;
        else
            *position = *position +1;
        if (*position >= procs_count)
          *position=0;
        if (temp >= procs_count){
            *proc_init_time = -1;
            return -1;
        }
        temp++;
      } while(1);
  }
  return -1;
}

int next_proc(proc_info_t *procs_info, int procs_count, int curr_time, struct Dupla *queue, int queue_procs_count,int *proc_init_time,int *position){
    int temp = 0, pos;

      pos = pos_pid(procs_info, procs_count, queue[*position].pid);
      if (!procs_info[pos].on_io && (curr_time - *proc_init_time < TIME_SLICE)){
            return queue[*position].pid;
      }
      else {
        do {
          *position = *position +1;
          if (*position >= queue_procs_count)
            *position=0;
          pos = pos_pid(procs_info, procs_count, queue[*position].pid);
          if (!procs_info[pos].on_io){
              *proc_init_time = curr_time;
              return queue[*position].pid;
          }
          if (temp >= queue_procs_count)
              return -1;
          temp++;
        } while(1);
      }

}

int MLFQ_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    int hp_pos, mp_pos, lp_pos; //posicion del preceso en cada una de las colas
    int temp_pos;
    /*
    Primero hay que eliminar de las colas aquellos procesos que ya terminaron y que no van a seguir ejecutandose
    */
    for (int i = 0; i < hp_procs_count;i++){//para cada uno de los procesos en la cola de alta prioridad
        temp_pos = pos_pid(procs_info, procs_count, hp_queue[i].pid);//lo busco en los procesos que me pasas
        if (temp_pos == -1){//si no esta, significa que ya termino y entonces lo borro de mi cola
          del_pid(hp_queue, hp_procs_count,i);
          hp_procs_count--;  
        }      
    }
    //lo mismo para la de prioridad media
    for (int i = 0; i < mp_procs_count;i++){
        temp_pos = pos_pid(procs_info, procs_count, mp_queue[i].pid);
        if (temp_pos == -1){
          del_pid(mp_queue, mp_procs_count,i);
          mp_procs_count--;  
        }      
    }
    //lo mismo para la de prioridad baja
    for (int i = 0; i < lp_procs_count;i++){
        temp_pos = pos_pid(procs_info, procs_count, lp_queue[i].pid);
        if (temp_pos == -1){
          del_pid(lp_queue, lp_procs_count,i);
          lp_procs_count--;  
        }      
    }

    if(curr_pid!=-1)
    {
      hp_pos = pos_queue(hp_queue, hp_procs_count,curr_pid);
      mp_pos = pos_queue(mp_queue, mp_procs_count,curr_pid);
      if(hp_pos!=-1)
      {
        //printf("alta %d \n", curr_pid);
        hp_queue[hp_pos].time_remaining+=10;
        if(hp_queue[hp_pos].time_remaining>=TIME_SLICE)
          {  
            mp_queue[mp_procs_count].pid=hp_queue[hp_pos].pid;
            mp_queue[mp_procs_count].time_remaining =0;
            mp_procs_count++;

            del_pid(hp_queue, hp_procs_count,hp_pos);
            hp_procs_count--;
          }
      }
      else if(mp_pos!=-1)
      {
        //printf("media %d \n", curr_pid);
        mp_queue[mp_pos].time_remaining+=10;
        if(mp_queue[mp_pos].time_remaining>=TIME_SLICE)
          {  
            lp_queue[lp_procs_count].pid=mp_queue[mp_pos].pid;
            lp_queue[lp_procs_count].time_remaining =0;
            lp_procs_count++;

            del_pid(mp_queue, mp_procs_count,mp_pos);
            mp_procs_count--;
          }
      }
    }
    /*Comprobando si en el momento actual se debe efectuar un Priority boost */
    if(curr_time % PRIORITY_BOOST ==0)
    {
      printf("PRIORITY_BOOST\n");
      hp_procs_count = mp_procs_count = lp_procs_count = 0;
      
      for(int j=0;j< procs_count;j++)
      {
        hp_queue[j].pid=procs_info[j].pid;
        hp_queue[j].time_remaining =0;
        hp_procs_count++;
      }
    }
    else
    {
      for(int i=0;i<procs_count;i++)
      {
        hp_pos=pos_queue(hp_queue,hp_procs_count,procs_info[i].pid);
        mp_pos=pos_queue(mp_queue,mp_procs_count,procs_info[i].pid);
        lp_pos=pos_queue(lp_queue,lp_procs_count,procs_info[i].pid);

        if(hp_pos < 0 && mp_pos < 0 && lp_pos < 0)
        {
          hp_queue[hp_procs_count].pid=procs_info[i].pid;
          hp_queue[hp_procs_count].time_remaining =0;
          hp_procs_count++;
        }
      }
    }
 
    /*Buscamos cada uno de los procesos en que cola se encuentra y le bajamos la prioridad si no esta en io o le subimos la prioridad si esta en io
    for (int i = 0; i < procs_count;i++){//para cada uno de los procesos
      //lo buscamos en las tres colas
      hp_pos = pos_pid(hp_queue, hp_procs_count,procs_info[i].pid);
      mp_pos = pos_pid(mp_queue, mp_procs_count,procs_info[i].pid);
      lp_pos = pos_pid(lp_queue, lp_procs_count,procs_info[i].pid);
      if (hp_pos>=0){//si esta en la de alta prioridad
          if (!procs_info[i].on_io){//y no esta haciendo io
              mp_queue[mp_procs_count]=procs_info[i];//lo pasamos a la cola de prioridad media
              mp_procs_count++;
              del_pid(hp_queue, hp_procs_count,hp_pos);
              hp_procs_count--;
          }
      }
      else if (mp_pos>=0){//si esta en la de prioridad alta
          if (!procs_info[i].on_io){//y no esta haciendo io
              lp_queue[lp_procs_count]=procs_info[i];//lo pasamos a la cola de prioridad baja
              lp_procs_count++;
              del_pid(mp_queue, mp_procs_count,mp_pos);
              mp_procs_count--;
          }
          else{//si esta en io, lo pasamos a la cola de prioridad alta
              hp_queue[hp_procs_count]=procs_info[i];
              hp_procs_count++;
              del_pid(mp_queue, mp_procs_count,mp_pos);
              mp_procs_count--;  
          }
      }
      else if (lp_pos>=0){//si esta en la de prioridad baja
          if (procs_info[i].on_io){//si esta en io, lo pasamos a la cola de prioridad alta
              hp_queue[hp_procs_count]=procs_info[i];
              hp_procs_count++;
              del_pid(lp_queue, lp_procs_count,lp_pos);
              lp_procs_count--;  
          }
      }
      else{//si no esta en ninguna de las colas, significa que es un proceso nuevo y lo ponemos en la cola de prioridad alta
          hp_queue[hp_procs_count]=procs_info[i];
          hp_procs_count++;
      }
    }*/
   /*
  printf("***hp_queue***\n");
    for (int i = 0; i < hp_procs_count;i++)
      printf("%d (%d)",hp_queue[i].pid,hp_queue[i].time_remaining );
    printf("\n");
    printf("***mp_queue***\n");
    for (int i = 0; i < mp_procs_count;i++)
      printf("%d (%d)",mp_queue[i].pid,mp_queue[i].time_remaining );
    printf("\n");
    printf("***lp_queue***\n");
    for (int i = 0; i < lp_procs_count;i++)
      printf("%d (%d)",lp_queue[i].pid,lp_queue[i].time_remaining );
    printf("\n");*/
    
    //scanf("%c");
    int nextp;
    //si hay procesos en la cola de prioridad alta, usamos rr
    if (hp_procs_count > 0){
        nextp = next_proc(procs_info, procs_count, curr_time, hp_queue, hp_procs_count,&hp_position, &hp_proc_init_time);
        if (nextp != -1)
            return nextp;
    }
    if (mp_procs_count > 0){
        nextp = next_proc(procs_info, procs_count, curr_time, mp_queue, mp_procs_count,&mp_position, &mp_proc_init_time);
        if (nextp != -1)
            return nextp;
    }
    if (lp_procs_count > 0){
        nextp = next_proc(procs_info, procs_count, curr_time, lp_queue, lp_procs_count,&lp_position, &lp_proc_init_time);
        if (nextp != -1)
            return nextp;
    }
    return -1;   
}

/*int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
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
}*/

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;
  if (strcmp(name, "sjf") == 0) return *SJF_scheduler;
  if (strcmp(name, "stcf") == 0) return *STCF_scheduler;
  if (strcmp(name, "rr") == 0) return *Round_Robin;
  if (strcmp(name, "mlfq") == 0) return *MLFQ_scheduler;
  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
