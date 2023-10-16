#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "simulation.h"

// int time_priority_boost = 100;

//FIFO(First In First Out)
int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

//SJF(Shortest Job First)
int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(procs_count <= 0)
  {
    return -1;
  }

  int minTime = INT_MAX;
  int pid;
  int duration;
  int pidFinal;

  for(int var=0; var<procs_count; var++)
  {
    pid = procs_info[var].pid;
    duration = process_total_time(pid); 
    if(pid == curr_pid)
    {
      return pid;
    }
    if(duration < minTime)
    {
      minTime = duration;
      pidFinal = pid;
    }
  }

  return pidFinal;
}

//STCF(Shortest Time-to-Completion First)
int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(procs_count <= 0)
  {
    return -1;
  }

  int minTimeRemaining = INT_MAX;
  int pid;
  int remaining_time;
  int pidFinal;

  for(int var=0; var<procs_count; var++)
  {
    pid = procs_info[var].pid;
    remaining_time = process_total_time(pid)-procs_info[var].executed_time;
    if(remaining_time < minTimeRemaining)
    {
      minTimeRemaining = remaining_time;
      pidFinal = pid;
    }
  }

  return pidFinal;
}

//Posicion en el array 'procs_info' que tenia el ultimo proceso que se ejecuto
int pos_last = 0;

//RR(Round Robin)
int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid) {
  if(procs_count <= 0)
  {
    return -1;
  }
  
  if(curr_pid != -1)
  {
    if(pos_last+1 >= procs_count)
    {
      pos_last = 0;
    }
    else
    {
      pos_last++;
    }
  }
  else
  {
    if (pos_last >= procs_count)
    {
      pos_last = 0;
    }
  }

  return procs_info[pos_last].pid;
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

  // if (strcmp(name, "mlfq") == 0) return *mlfq_scheduler;

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}

// //TRABAJANDO EN EL MLFQ

// /*Creando una estructura que va a representar cada proceso en mi cola. Creo tres variables que van a identificar las tres colas de prioridad para
// mi MLFQ.
// */
// typedef struct Process {
//   proc_info_t proc;
//   Process *next;
//   Process *previous;
// };

// //Creando los punteros root y final que apuntan respectivamente al inicio y final de las colas 
// Process* root[3] = {NULL, NULL, NULL};
// Process* final[3] = {NULL, NULL, NULL};

// //Inserta un proceso a la cola especificada
// void insert(proc_info_t x, Process *root, Process *final)
// {
//   Process *new;
//   new = malloc(sizeof(Process));
//   new->proc = x;
//   new->next = NULL;
//   new->previous = final;
//   if (root == NULL)
//   {
//     root = new;
//     final = new;
//   }
//   else
//   {
//     final->next = new;
//     final = new;
//   }
// }

// //Obteniendo el proceso que esta en CPU
// Process* get_proc(Process *root)
// {
//   Process *bor = root;
//   while(bor!=NULL && (bor->proc).on_io)
//   {
//     bor = bor->next;
//   }

//   return bor;
// }
// //Extraer un proceso de la cola
// proc_info_t extract(Process *root,  Process *final, Process *bor)
// {
//   //Si no esta en la cola o todos los procesos estan en IO
//   if(bor==NULL)
//   {
//     return NULL;
//   }
  
//   if(bor == root)//Si el proceso es el principio de la cola
//   {
//     if (root == final) //Un solo elemento en la cola
//     {
//       root = NULL;
//       final = NULL;
//     }
//     else //Mas de un elemento en la cola
//     {
//       (root->next)->previous = NULL;
//       root = root->next;
//     }
//   }
//   else if(bor == final)//Si el proceso es el final de la cola
//   {
//     (final->previous)->next = NULL;
//     final = final->previous;
//   }
//   else //Si el proceso esta en el medio de la cola
//   {
//     (bor->previous)->next = bor->next;
//     (bor->next)->previous = bor->previous;
//   }

//   proc_info_t info = bor->proc;
//   free(bor);
//   return info;
// }

// //Buscar un proceso en una cola
// int search (proc_info_t *procs_info, Process *root)
// {
//   Process *pointer;
//   for(int i=0; i<3; i++)
//   {
//     pointer = root[i];
//     while(pointer!=NULL)
//     {
//       if(proc_info==pointer) return 1;
//       pointer = pointer->next;
//     }
//   }

//   return 0;
// }

// //Variables para el MLFQ
// proc_info_t last_proc_exec;
// int number_queue_last_proc;

// //MLFQ (Multi-level Feedback Queue)
// int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid) {
//   if(procs_count == 0)
//   {
//     return -1;
//   }

//   //1.Procesos nuevos
//   for(int k=0; k<procs_count; k++)
//   {
//     if(!search(procs_info[k], root))
//     {
//       insert(procs_info[k], root[0], final[0]);
//     }
//   }

//   //2.Bajarle la prioridad si el proceso no ha terminado
//   if(curr_pid != -1)
//   {
//     if(number_queue_last_proc < 2)
//     {
//       insert(last_proc_exec, root[number_queue_last_proc+1], final[number_queue_last_proc+1]);
//     }
//     else{
//       insert(last_proc_exec, root[number_queue_last_proc], final[number_queue_last_proc]);
//     }
//   }

//   //3.Poniendo cada cierto tiempo 'time_priority_boost' todos los procesos en la cola de mayor prioridad
//   if(curr_time%time_priority_boost == 0)
//   {
//     proc_info_t change_priority;
//     for(int j=2; j>0; j--)
//     {
//       while(root[j]!=NULL)
//       {
//         change_priority = extract(root[i], final[i], root[i]);
//         insert(change_priority, root[0], final[0]);
//       }
//     }
//   }

//   //4.Extraer proceso de mayor prioridad
//   int i=0;
//   do{
//     last_proc_exec = extract(root[i], final[i], get_proc(root[i]));
//     number_queue_last_proc = i;
//     i++;
//   }while(last_proc_exec==NULL && i<3);
  
//   //5.Ejecutar
//   if(last_proc_exec==NULL)
//   {
//     number_queue_last_proc = -1;
//     return -1;
//   } 

//   return last_proc_exec.pid;
// }







