#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "simulation.h"
/*MFLQ*/
#define TIME_SLICE 50
#define PRIORITY_BOOST 500

struct Dupla
{
  int pid;//Pid del proceso
  int time_remaining;//Tiempo de ejecución que le queda en una cola  
};


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

/*Round Robin*/
int position = 0;
int proc_init_time;

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



/*MLFQ*/
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

//devuelve la posicion de un proceso en el procs_info, si no esta devuelve -1
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

//devuelve la posición de un proceso en la cola,si no se encuentra devuelve -1
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

int next_proc(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid, struct Dupla *queue, int queue_procs_count,int *proc_init_time,int *position){
    int temp = 0, pos;
      
      //printf("retorno1 %d \n", queue[*position].pid);

      //buscamos en procs_info el último proceso de esta cola que fue ejecutado
      pos = pos_pid(procs_info, procs_count, queue[*position].pid);
      //printf("pos %d \n", pos);
      
      //si no está en I/O y su time slice no se ha consumido,se retorna ese proceso
      if (pos != -1 && !procs_info[pos].on_io && (curr_time - *proc_init_time < TIME_SLICE)){
            
            //printf("retorno %d %d \n", procs_info[pos].pid,queue[*position].pid);
            return queue[*position].pid;
      }
      else {//sino,se busca por toda la cola el siguiente proceso que no se encuentra en I/O
        do {
          *position = *position +1;
          if (*position >= queue_procs_count)
            *position=0;
          pos = pos_pid(procs_info, procs_count, queue[*position].pid);
          if (!procs_info[pos].on_io){
              *proc_init_time = curr_time;
              return queue[*position].pid;
          }
          if (temp >= queue_procs_count)//en caso de que la cantidad de iteraciones pase el tamaño 
            return -1;//de la cola significa que todos los procesos estan en I/O y se retorna -1
              

          temp++;
        } while(1);
      }

}

int MLFQ_scheduler(proc_info_t *procs_info, int procs_count, int curr_time, int curr_pid) {
    int hp_pos, mp_pos, lp_pos; //posicion del preceso en cada una de las colas
    int temp_pos;
   
    //Primero hay que eliminar de las colas aquellos procesos que ya terminaron 
    //y que no van a seguir ejecutandose
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

    //compruebo si el último proceso que se ejecutó existe todavía
    if(curr_pid!=-1)
    { //lo busco tanto en la cola de alta prioridad como el da de media
      hp_pos = pos_queue(hp_queue, hp_procs_count,curr_pid);
      mp_pos = pos_queue(mp_queue, mp_procs_count,curr_pid);
      if(hp_pos!=-1)//si está en la cola de alta prioridad aumento el time_remaining y compruebo
      {            //si ya consumí todo su time_slice,en caso de ocurrir,lo paso a la cola media

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
      else if(mp_pos!=-1)//mismo proceso pero desde la cola de prioridad media a la baja
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
      /*
      printf("******************PRIORITY_BOOST**************************\n");
      printf("******************PRIORITY_BOOST**************************\n");
      printf("******************PRIORITY_BOOST**************************\n");
      printf("******************PRIORITY_BOOST**************************\n");
      */
      hp_procs_count = mp_procs_count = lp_procs_count = 0;//al hace cola_procs_count = 0 estamos
                         // convirtiendo todos los valores que se encontraban en datos no válidos
      
      for(int j=0;j< procs_count;j++)//agrego todos los procesos del procs_info a 
                                    //la cola de prioridad alta
      {
        hp_queue[j].pid=procs_info[j].pid;
        hp_queue[j].time_remaining =0;
        hp_procs_count++;
      }
      hp_position = 0;
    }
    else //sino compruebo si existe algún proceso nuevo y lo agrego a la cola de mayor prioridad
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
    printf("\n");
    for(int i = 0; i < 10000000;i++);*/
   
   /*Para elegir el siguiente proceso se utiliza next_proc,en caso de que todos los procesos de
    una cola estén en I/O next_proc devuelve -1,por lo que se pasa a analizar la siguiente cola.
    En caso de que todos los procesos estén en I/O el scheduler retorna -1. */
    int nextp;
    if (hp_procs_count > 0){
        nextp = next_proc(procs_info, procs_count, curr_time, curr_pid, hp_queue, hp_procs_count,&hp_position, &hp_proc_init_time);
        if (nextp != -1)
            return nextp;
    }
    if (mp_procs_count > 0){
        nextp = next_proc(procs_info, procs_count, curr_time, curr_pid,mp_queue, mp_procs_count,&mp_position, &mp_proc_init_time);
        if (nextp != -1)
            return nextp;
    }
    if (lp_procs_count > 0){
        nextp = next_proc(procs_info, procs_count, curr_time, curr_pid, lp_queue, lp_procs_count,&lp_position, &lp_proc_init_time);
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
