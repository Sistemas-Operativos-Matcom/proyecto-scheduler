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
  //si el proceso actual no ha terminado de ejecutarse devolvemos su pid
  if(curr_pid != -1) return curr_pid;

  //si el ultimo proceso termino su ejecucion, buscamos en la lista de procesos activos
  //el proceso de menor ejecucion y devolvemos su pid
  int min_pid = procs_info[0].pid;
  int min_time = process_total_time(procs_info[0].pid);
  for(int i = 1; i< procs_count; i++){
    if(process_total_time(procs_info[i].pid)< min_time){
      min_time = process_total_time(procs_info[i].pid);
      min_pid = procs_info[i].pid;
    }
  }
  return min_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //si tenemos un proceso en ejecucion buscamos su pid y cuanto le queda por ejecutar
  //esto lo hacemos para si el proceso actual es el que menor tiempo le queda 
  //y hay otro que le queda el mismo tiempo, devolver el proceso actual para evitar cambios
  //de contextos innecesarios

  int actual_procs = -1;
  for(int i = 0; i< procs_count; i++){
    if(procs_info[i].pid == curr_pid){
      actual_procs = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
    }
  }

  //buscamos al proceso que le queda menos tiempo de ejecucion y guardamos su pid
  int min_time_finish = process_total_time(procs_info[0].pid) - procs_info[0].executed_time;
  int min_pid_finish = procs_info[0].pid;
  for(int i = 1; i<procs_count; i++){
    if(process_total_time(procs_info[i].pid) - procs_info[i].executed_time  < min_time_finish){
      min_time_finish = process_total_time(procs_info[i].pid) - procs_info[i].executed_time;
      min_pid_finish = procs_info[i].pid;
    }
  }
  
  //comparamos los tiempo de ejecucion de el proceso actual y el proceso minimo
  //si son iguales, devolvemos el proceso actual
  if(actual_procs == min_time_finish) return curr_pid;
  return min_pid_finish;
}

static int position = 0;
static int time_slice = 0;

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  //si el proceso actual termino, ejecutamos el siguiente proceso
  //y reiniciamos el time slice
  if(curr_pid == -1){
    time_slice = 0;
    //si el ultimo proceso en ejecutar fue el ultimo, empezamos a ejecutar el primero
      if(position == procs_count){
        position = 0;
        return procs_info[position].pid;
      }
      //si el proceso anterior termino, el array de procesos se corre 
      //y el proceso siguiente queda en la misma posicion
      return procs_info[position].pid;
  }
  //si llegamos al time slice buscamos el proceso para devolverlo          
  if(time_slice == 5){
    time_slice = 0;
    //si el ultimo proceso ejecutado es el ultimo del array, reiniciamos position y devolvemos el primer pid
    if(position +1 == procs_count){
      position = 0;
      return procs_info[position].pid;
    }
    //sino devolvemos el siguiente proceso
    position +=1;
    return procs_info[position].pid;
  }
  
  //si el time slice es menor que el definido (50ms) lo actualizamos
  //y devolvemos el proceso que estaba en ejecucion
  if(time_slice<5){
      time_slice ++;
      return curr_pid;
  }
  return 0;
}

static int position_0 = 0; //posicion del ultimo proceso ejecutado en prioridad 0
static int position_1 = -1; //posicion del ultimo proceso ejecutado en prioridad 1
static int position_2 = -1; ///posicion del ultimo proceso ejecutado en prioridad 2
static int ult_pr = 0; //prioridad del ultimo proceso ejecutado
static int time_slice_mlfq = 0; //time slice para rr
static int time_priority_boost = 0; //tiempo para hacer priority boost
static int next_procs = 0; //posicion del proceso siguiente

void Update_Pos(int priority, int procs_count, proc_info_t *procs_info){
  //actualizar cada puntero de la cola de cada prioridad si el proceso anterior termino y estaba antes de un puntero
  int curr_position = 0;
  int pos_1 = 0;
  int pos_2 = 0;
  if(priority == 0){
    curr_position = position_0;
    pos_1 = position_1;
    pos_2 = position_2;
  }
  if(priority == 1){
    curr_position = position_1;
    pos_1 = position_2;
    pos_2 = position_0;
  }
  if(priority == 2){
    curr_position = position_2;
    pos_1 = position_1;
    pos_2 = position_0;
  }
  
  //disminuimos uuno en la posicion de cada puntero si el proceso que termino estaba antes de cada puntero
  if(pos_1 > curr_position) pos_1 -=1;
  if(pos_2 > curr_position) pos_2 -=1;

  //actualizamos el puntero de la cola de la prioridad del proceso que termino 
  //buscando primero si hay un proceso de su misma prioridad en la cola despues que el
  //si este proceso no existe buscamos en la cola antes que el
  //si este ultimo no existe apuntamos a -1, pues no hay procesos con dicha prioridad
  int res =0;
  int f = 0;
  for(int i = procs_count -1; i>=0; i--){
    if(*procs_info[i].priority == priority && i>= curr_position){
      curr_position = i;
      res = 1;
      f =1;
    }
    if( i<curr_position && *procs_info[i].priority == priority && res ==0){
      curr_position =i;
      f=1;
    }
  }
  if(f == 0) curr_position =-1;

  if(priority == 0){
    position_0 = curr_position;
    position_1 = pos_1;
    position_2 = pos_2;
  }
  if(priority == 1){
    position_1 = curr_position;
    position_2 = pos_1;
    position_0 = pos_2;
  }
  if(priority == 2){
    position_2 = curr_position;
    position_1 = pos_1;
    position_0 = pos_2;
  }
}

int Next_Pid(int priority, int position, int procs_count, proc_info_t *procs_info){
  //dado una prioridad buscamos el posible pid que vamos a retornar 
  int res = 0;
  int pid = -1;
  int pos = -1;
  //buscamos el proceso siguiente despues de la posicion del proceso anterior
  //si no encontramos proceso con la misma prioridad buscamos un proceso que este en posicion anterior
  //si no existe devolvemos -1
  for(int i = procs_count-1; i>= 0; i--){
    if(*procs_info[i].priority == priority && i>= position && procs_info[i].on_io ==0){
     res =1;
     pid = procs_info[i].pid;
     pos = i;
    }
    if(i< position && *procs_info[i].priority == priority && procs_info[i].on_io ==0 && res ==0){
      pid = procs_info[i].pid;
      pos = i;
    }
  }
  //actualizmos la posicion del proceso nuevo, si no existe proceso nuevo, la posicion sera -1
  next_procs = pos;
  return pid;
}

int Next_Pid_Ret(int min_pr, int procs_count, proc_info_t *procs_info, int bool, int curr_pid){
  //buscamos el proceso a retornar 
  int aux = min_pr;
  int pos_to_pid = position_0;
  //buscamos un proceso en la maxima prioridad para retornar, si no existe aumentamos la prioridad y seguimos buscando
  while(aux <3){
    if(aux == 1) pos_to_pid = position_1;
    if(aux == 2) pos_to_pid = position_2;
    int next_pid_exec = Next_Pid(aux, pos_to_pid, procs_count, procs_info);
    if(next_pid_exec != -1) {
      if(aux == 0) position_0 = next_procs;
      if(aux == 1) position_1 = next_procs;
      if(aux == 2) position_2 = next_procs;
      //si encontramos un proceso a devolver actualizamos todos los parametros
      ult_pr = aux;
      time_priority_boost ++;
      if(time_slice_mlfq != 5){
        time_slice_mlfq ++;
        if(bool ==  0 && curr_pid != next_pid_exec) time_slice_mlfq =0;
      }
      else{
        time_slice_mlfq =0;
      }
      *procs_info[next_procs].time_in_CPU+=10; 
      return next_pid_exec;
    }
    aux ++;
  }
  //si no existe proceso a devolver, o todos estan en I/O devolvemos -1
  return -1;
}

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  int min_pr = 3;
  //si llegamos a 300ms hacemos priority boos en todos los procesos y ponemos el tiempo en cpu de cada proceso en 0
  if(time_priority_boost == 30){
    time_priority_boost = 0;
    for(int i = 0; i < procs_count; i++){
      *procs_info[i].priority = 0;
      *procs_info[i].time_in_CPU = 0;
    }
    //actualizamos los punteros de cada cola y la ultima prioridad ejecutada
    position_1 = -1;
    position_2 = -1;
    min_pr = 0;
    ult_pr = 0;
  }
  else{
    //buscamos cual es la menor prioridad existente
    for(int i = 0; i<procs_count; i++){
      if(*procs_info[i].priority == 0 && position_0 == -1) position_0 = i;
      if(*procs_info[i].priority < min_pr){
        min_pr = *procs_info[i].priority;
      }
    }
  }
  //si el proceso anterior termino
  if(curr_pid == -1){
    //actualizar los punteros de cada prioridad
    Update_Pos(ult_pr, procs_count, procs_info);
    //y buscamos el proximo proceso a devolver por la minima prioridad existente
    return Next_Pid_Ret(min_pr, procs_count, procs_info, 0, curr_pid);
  }
  //si el proceso anterior no termino
  for(int i = 0; i< procs_count; i++){
    //buscamos el proceso que se estaba ejecutando
    if(procs_info[i].pid == curr_pid){
      //si su tiempo en CPU llego a 100ms bajamos su prioridad
      if(*procs_info[i].time_in_CPU == 100){
        *procs_info[i].time_in_CPU = 0;
        int res =0;
        int pos = -1;
        //y buscamos un proceso de su misma prioridad inicial para cambiar el puntero
        //si no existe se guardara -1
        for(int j= procs_count-1; j>=0; j--){
          if(*procs_info[i].priority == *procs_info[j].priority && j>i){
            res =1;
            pos =j;
          }
          if(*procs_info[i].priority == *procs_info[j].priority && j<i && res ==0){
            pos = j;
          }
        }
        if(*procs_info[i].priority ==0){
          position_0 = pos;
          if(position_1 == -1) position_1 = i;
        }
        if(*procs_info[i].priority ==1){
          position_1 = pos;
          if(position_2 == -1) position_2 = i;
        }   
        //le aumentamos en uno a la prioridad si es distinta de 2
        //si la prioridad es 2, se le mantiene       
        if(*procs_info[i].priority !=2) *procs_info[i].priority = *procs_info[i].priority+1;
        //y buscamos el siguiente proceso a devolver
        return Next_Pid_Ret(min_pr, procs_count, procs_info, 0, curr_pid);
      }
      //si el proceso actual no ha llegado a 100ms en CPU
      else{
        //y esta haciendo una operacion de IO, devolvemos el proceso segun la minima prioridad que nos encontremos
        if(procs_info[i].on_io == 1){
          return Next_Pid_Ret(min_pr, procs_count, procs_info, 0, curr_pid);
        }
        //si no esta haciendo una operacion IO
        int res = 0;
        int pos = -1;
        //buscamos otro proceso de su mimsa prioridad para cambiar el puntero
        for(int j = procs_count-1; j>=0; j--){
          //buscamos un proceso que este despues del ultimo y si no lo encontramos
          if(*procs_info[i].priority == *procs_info[j].priority && j>i){
            res =1;
            pos =j;
          }
          //buscamos un proceso que este antes
          if(*procs_info[i].priority == *procs_info[j].priority && j<i && res ==0){
            pos = j;
          } 
          //si no existe se guardara -1
        }
        if(time_slice_mlfq == 5){
          //si el time slice llego a 50ms significa que tenemos que cambiar de proceso
          //actualizamos la posicion del puntero de la prioridad del proceso anterior
          if(pos != -1){
            if(*procs_info[i].priority == 0) position_0 = pos;
            if(*procs_info[i].priority == 1) position_1 = pos;
            if(*procs_info[i].priority == 2) position_2 = pos;
          }
          //devolvemos el siguiente proceso una vez actualizados los punteros
          return Next_Pid_Ret(min_pr, procs_count, procs_info, 0, curr_pid);
        }
        //si no se ha consumido el tiempo del time slice buscamos el siguiente proceso y lo devolvemos
        else{
          return Next_Pid_Ret(min_pr, procs_count, procs_info, 1, curr_pid);
        }
          
      }
    }
  }
  
  return 0;
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

  // Añade aquí los schedulers que implementes. Por ejemplo:
  //
  // if (strcmp(name, "sjf") == 0) return *sjf_scheduler;
  //

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}
