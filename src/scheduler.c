#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

int act_proc=-1;

int *pid_save;

int pid_save_size = 0;

int s=5;


int check_PID(proc_info_t *procs_info, int count, int act_proc_PID){

for (size_t i = 0; i < count; i++)
{
  if(procs_info[i].pid==act_proc_PID){
    return 1;
  }
}

return 0;

}

void sort(proc_info_t *procs_info,int procs_count){

for (size_t i = 0; i < procs_count; i++)
       {
        for (size_t j = i+1; i < procs_count; j++)
        {
          if( process_total_time(procs_info[i].pid)> process_total_time(procs_info[j].pid)){
            proc_info_t swap = procs_info[i];
            procs_info[i]=procs_info[j];
            procs_info[i]=swap;
          }
        }
        
       }


}


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

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid){

if (check_PID(procs_info,procs_count,act_proc))
{
  return act_proc;
}

// proc_info_t *procs_info_copy =procs_info;

//        for (size_t i = 0; i < procs_count; i++)
//        {
//         for (size_t j = i+1; i < procs_count; j++)
//         {
//           if( process_total_time(procs_info_copy[i].pid)> process_total_time(procs_info_copy[j].pid)){
//             proc_info_t swap = procs_info_copy[i];
//             procs_info_copy[i]=procs_info_copy[j];
//             procs_info_copy[i]=swap;
//           }
//         }
        
//        }

sort(procs_info,procs_count);

act_proc=procs_info[0].pid;
return act_proc;


}


int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid){

      //  proc_info_t *procs_info_copy =procs_info;

      //  for (size_t i = 0; i < procs_count; i++)
      //  {
      //   for (size_t j = i+1; i < procs_count; j++)
      //   {
      //     if( process_total_time(procs_info_copy[i].pid)> process_total_time(procs_info_copy[j].pid)){
      //       proc_info_t swap = procs_info_copy[i];
      //       procs_info_copy[i]=procs_info_copy[j];
      //       procs_info_copy[i]=swap;
      //     }
      //   }
        
      //  }

sort(procs_info,procs_count);

       return procs_info[0].pid;
           
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid){



int* aux=malloc(pid_save_size * sizeof(int));

for (size_t i = 0; i < pid_save_size; i++)
{
  aux[i]=pid_save[i];
}

pid_save= malloc(procs_count * sizeof(int));
int aux_size= pid_save_size;
pid_save_size=procs_count;

for (size_t i = 1; i < aux_size; i++)
{
  pid_save[i-1]=aux[i];
}


for (size_t i = 0; i < procs_count; i++)
{
  int is=0;

  for (size_t j = 0; i < pid_save_size; i++)
  {
    if(procs_info[i].pid==pid_save[j]){

      is=1;
      break;

    }
  }

  if (is==0){

  pid_save[aux_size]=procs_info[i].pid;
  aux_size++;

  }

  return pid_save[0];
    
}
  
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
