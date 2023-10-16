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
                   int curr_pid) 
{
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

// int fifo_cpu_time[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// int get_curr_index(proc_info_t *procs_info, int procs_count, int curr_pid)
// {
//   for (int i = 0; i<procs_count; i++)
//   {
//     if (procs_info[i].pid == curr_pid)
//       return i;
//   }
//   return -1;
// }

// int my_own_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
//                      int curr_pid) {
  // Implementa tu scheduler aqui ... (el nombre de la función lo puedes
  // cambiar)
  
  // Información que puedes obtener de un proceso
  // int pid = procs_info[0].pid;      // PID del proceso
  // int on_io = procs_info[0].on_io;  // Indica si el proceso se encuentra
  //                                    realizando una opreación IO
  // int exec_time = procs_info[0].executed_time;  // Tiempo que el proceso se ha
  //                                               ejecutado (en CPU o en I/O)
  
  // También puedes usar funciones definidas en `simulation.h` para extraer
  // información extra:
  // int duration = process_total_time(pid);
//   int ind = get_curr_index(procs_info, procs_count, curr_pid);
  
//   printf("%d\n", fifo_cpu_time[curr_pid]);
//   if (procs_info[ind].on_io == 0)
//   {
//     fifo_cpu_time[curr_pid] += 10;
//   }
//   else
//   {
//     if (procs_count>1)
//       return procs_info[1].pid; 
//   }
//   return procs_info[0].pid;
// }

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int sdur = __INT_MAX__;
  int spid = -1;
  
  //Recorre el array para determinar el proceso al que le queda menos.
  for (int i = 0; i < procs_count; i++)
  {
    int cpid = procs_info[i].pid;
    int duration = process_total_time(cpid);
    
    if (duration < sdur)
    {
      sdur = duration;
      spid = cpid;
    }
  }
  return spid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  //Si no ha terminado el proceso, seguir ejecutándolo.
  if (curr_pid != -1)
    return curr_pid;
  
  int sdur = __INT_MAX__;
  int spid = -1;

  for (int i = 0; i < procs_count; i++)
  {
    int cpid = procs_info[i].pid;
    int duration = process_total_time(cpid);
    if (duration < sdur)
    {
      sdur = duration;
      spid = cpid;
    }
  }
  return spid;
}

int time_slice = 50; 
int rr_index = 0;

//Devuelve 1 si el proceso es el ultimo proceso; 0 en caso contrario
int set_last(int rr_index, int procs_count)
{
  if (rr_index >= procs_count-1)
    return 1;
  else
    return 0;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{
  int last = set_last(rr_index, procs_count);

  //Tiempo que se ha ejecutado un proceso
  int exec_time = procs_info[rr_index].executed_time;

  //Caso inicial. Como rr_index inicial es 0, si el proceso 0 no se ha ejecutado, comenzar a ejecutarlo.
  if (exec_time == 0)
    return procs_info[rr_index].pid;

  //Si se cumple, no ha llegado al time slice y se sigue ejecutando el mismo proceso
  if (exec_time % time_slice != 0)
  {
    if (curr_pid != -1)
      return curr_pid;
  }

  //Si no hay un proceso siguiente, comienza por el proceso 0.
  //De lo contrario, devuelve el pid del siguiente proceso.
  if (last != 1)
  {
    rr_index++;
    return procs_info[rr_index].pid;   
  }
  rr_index = 0;
  return procs_info[rr_index].pid;
}

//--------------------------------    INICIALIZACIONES DE MLFQ -------------------------------------

//Cola 0 menor prioridad, cola 2 mayor prioridad
  proc_info_t q0[200];
  proc_info_t q1[200];
  proc_info_t q2[200];

  proc_info_t *queues[3] = {q0, q1, q2};

  //Define la cola del proceso que se está ejecutando:
  int active_queue = -1;
  
  //Array con los pids de los procesos añadidos. Size = 200
  //added[i] = 1 --> proceso con pid = i añadido
  int added[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  //Almacena el tiempo que lleva ejecutándose en cpu un proceso (aproximado)
  //cpu_time[i] = x --> proceso con pid = i lleva x ms ejecutándose en cpu
  int cpu_time[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  //Priority boost timer. Define cada cuanto tiempo realizar un priority boost
  int pb_timer = 500;
  //Me indica si se ha ejecutado o no algún proceso con mlfq. 0 es que no
  int mlfq_begin = 0;


// -------------------------  METODOS NECESARIOS PARA IMPLEMENTAR MLFQ---------------------------

//Si un proceso terminó, devuelve 1, Else devuelve 0
//Recibe el pid del proceso, la lista y la cantidad de procesos actuales activos
int process_ended(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i<procs_count; i++)
  {
    if (procs_info[i].pid == pid)
      return 0;
  }
  return 1;
}

//Retorna el proceso buscado
proc_info_t get_process(proc_info_t *procs_info, int procs_count, int c_pid)
{
    for (int i = 0; i<procs_count; i++)
    {
      if (procs_info[i].pid == c_pid)
        return procs_info[i];
    }
    proc_info_t error = {-1, -1, -1};
    printf("Se produjo un error");
    return error;
}

//Devuelve si el proceso esta en io en tiempo real
int get_io(proc_info_t *procs_info, int procs_count, int pid)
{
  for (int i = 0; i<procs_count; i++)
  {
    if (procs_info[i].pid == pid)
      return procs_info[i].on_io;
  }
  printf("Error. No encontrado");
  return -1;
}

//Devuelve si en la cola actual quedan procesos por ejecutarse que no esten en IO (1--> si quedan)
int queue_not_empty(proc_info_t *current_queue, proc_info_t *procs_info, int procs_count)
{
  for (int i = 0; i<200; i++)
  {
    int pid = current_queue[i].pid;
    if (pid == 0)
    {       
      if (
          (current_queue[i].executed_time != 0) &&                // Si es el proceso 0
          (process_ended(procs_info, procs_count, pid) == 0) &&   // y no se ha terminado
          (get_io(procs_info, procs_count, pid) == 0)             // y no está haciendo IO
         )
        return 1;                                                 // devuelve No vacío
    }
    else  //Si no es el proceso cero:
    {
      //Devuelve 1 si hay algun proceso no terminado y que no está haciendo IO
      if (
        (process_ended(procs_info, procs_count, pid) == 0)  //El proceso no ha terminado
        && 
        (get_io(procs_info, procs_count, pid) == 0)        //El proceso no está haciendo IO
        )   
        return 1;
    }      
  }
  return 0;
}

//De los procesos activos, añadir los que no están en alguna cola a la cola de mayor prioridad (la cola 2):
void add_to_queue(proc_info_t *procs_info, int procs_count)
{
  for (int i = 0; i < procs_count; i++)
  {
    int index = procs_info[i].pid;
    if (added[index] == 0)    //el proceso no está en cola
    {
      added[index] = 1;       //marcarlo como añadido
      q2[index] = procs_info[i];  //añadirlo a la cola 2
    }
  }
}

//Disminuye la prioridad del proceso que se está ejecutando porque consumió el time slice
void decrease_priority(int curr_pid, proc_info_t curr_process)
{
  //Quitar el proceso de la cola actual:
  proc_info_t empty_process = {0,0,0};
  queues[active_queue][curr_pid] = empty_process;

  //Mover el proceso a la cola de abajo
  queues[active_queue - 1][curr_pid] = curr_process;

  //printf("Curr proc: %d, exec time: %d\n", curr_process.pid, curr_process.executed_time);
}

//Incrementa la prioridad de todos los procesos
void priority_boost(proc_info_t *procs_info, int procs_count)
{
  //printf("REALIZADO PB\n");
  proc_info_t empty_process = {0,0,0};
  
  //Vaciar todas las colas
  for (int i = 0; i<3; i++)
  {
    for (int j = 0; j<200; j++)
    {
      queues[i][j] = empty_process;
    }
  }

  //Poner todos los procesos en la cola de mayor prioridad
  for (int i = 0; i < procs_count; i++)
  {
    queues[2][procs_info[i].pid] = procs_info[i];
  }
  active_queue = 2;
}

//Devuelve 1 si es el último proceso de la cola, 0 en caso contrario.
int is_last(proc_info_t *queue, int curr_pid)
{
  for (int i = curr_pid+1; i<200; i++)
  {
    if (queue[i].pid != 0)
      return 0;
  }
  return 1;
}

//En cada time interrupt, aumenta en 10 el cpu_time del proceso que se va a retornar
void inc_cpu_time(int pid)
{
  cpu_time[pid] += 10;
}

//Implementacion de round robin para MLFQ
int mlfq_rr(proc_info_t *queue, proc_info_t *procs_info, int procs_count, int queue_num, int curr_pid, int prev_q, int curr_time)
{
  if (curr_time == 19600)
  {
      printf("\n\nCola actual: %d\n\n",active_queue);
  }
  //Almacena si el proceso actual es el último de su cola.
  int last = is_last(queue, curr_pid);

  for (int i = 0; i < 200; i++)
  {
    int pid = queue[i].pid; 
    if (pid != 0 || queue[i].executed_time != 0)
    {
      //Si el proceso no ha terminado, lo devuelve
      if (process_ended(procs_info, procs_count, pid) == 0) 
      {
        //Si es la última cola, asegurarme de no hacer completo el mismo proceso, a menos que no haya otro pendiente en la última cola
        if (queue_num == 0)   
        {
          //Si el proceso actual no es el último proceso de la cola
          if (last == 0)
          {
            //Hay un proceso siguiente en cola que debe ejecutarse, cuyo index=pid es mayor.
            if (pid > curr_pid)
            {
              //Si el proceso no está en io
              if (get_io(procs_info, procs_count, i) == 0)
              {
                inc_cpu_time(pid);  //Aumentar su tiempo en el cpu por los siguientes 10ms
                return pid;
              }              
            }
          }
          else  //Es el último proceso de su cola, hay que empezar de nuevo.
          {
            //Si el proceso no está en io
            if (get_io(procs_info, procs_count, i) == 0)
            {
              inc_cpu_time(pid);  //Aumentar su tiempo en el cpu por los siguientes 10ms
              //Devolver el primer proceso sin terminar de la cola que me encuentre
              return pid;
            }   
          }
        }
        else
          {  
            //Si el proceso no está en io
            if (get_io(procs_info, procs_count, i) == 0)
            {
              inc_cpu_time(pid);  //Aumentar su tiempo en el cpu por los siguientes 10ms
              return pid;
            }    
          }
      }        
    }
  }
  //Si llega hasta aquí es porque está en la última cola y no hay ningún proceso pendiente además del actual, por tanto, hay que devolver el actual.
  
  //Si el proceso actual no está en io
  if (get_io(procs_info, procs_count, curr_pid) == 0)
  {
    inc_cpu_time(curr_pid);   //Aumentar su tiempo en el cpu por los siguientes 10ms
    active_queue = prev_q;    //Al llamar al rr se cambio el active queue, asi que se vuelve a setear
                              //al que habia antes
    return curr_pid;
  }
  //El proceso actual está en io.
  return -1; 
}

//Devuelve el siguiente proceso a ser ejecutado
int next_process(proc_info_t *procs_info, int procs_count, int curr_pid, int curr_time)
{
  //Se empieza por la cola de mayor prioridad y se va bajando hasta encontrar una cola con un proceso pendiente.
  for (int i = 2; i>=0; i--)
  {
    //Si quedan procesos que no estan en io
    if (queue_not_empty(queues[i], procs_info, procs_count) == 1)   
    {
      int prev_q = active_queue;
      active_queue = i;
      return mlfq_rr(queues[i], procs_info, procs_count, i, curr_pid, prev_q, curr_time);
    }      
  }
  //printf("ERROR GRAVE 1");
  //Solo queda un proceso pendiente y está realizando IO
  inc_cpu_time(curr_pid);
  return curr_pid;   
}

// -----------------------------------  -->     MLFQ   <--  ------------------------------------

int mlfq_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                     int curr_pid)
{

  //Debugger
  int chkT = -19600;
  if (curr_time == chkT)
  {
    //proc_info_t curr = get_process(procs_info, procs_count, curr_pid);
    printf("active queue: %d; curr_pid: %d\n", active_queue, curr_pid);
    printf("Cola 2:\n");
    for (int i = 5; i<10; i++)
    {
      printf(" P:%d", queues[2][i].pid);
    }
    printf("\nCola 1:\n");
    for (int i = 5; i<10; i++)
    {
      printf(" P:%d", queues[1][i].pid);
    }
    printf("\nCola 0:\n");
    for (int i = 5; i<10; i++)
    {
      printf(" P:%d", queues[0][i].pid);
    }
    printf("\n");

    //printf("CurrPid ON IO: %d \n", curr.on_io);
  }

  //Realizar Priority Boost
  if (curr_time % pb_timer == 0)
    priority_boost(procs_info, procs_count);
  
  //Punto de partida para mlfq. Ejecuta siempre el primer proceso de todos.
  if (mlfq_begin == 0)
  {
    mlfq_begin = 1;
    added[0] = 1;
    q2[0] = procs_info[0];
    active_queue = 2;
    inc_cpu_time(0);
    return procs_info[0].pid;
  }

  //Añadir los procesos que van llegando a la cola de mayor prioridad
  add_to_queue(procs_info, procs_count);  

  //Si hay algun proceso ejecutandose, seguir ejecutandolo hasta que consuma el time slice:
  if (curr_pid != -1)
  {
    //Obtengo el proceso actual
    proc_info_t curr_process = get_process(procs_info, procs_count, curr_pid);
    
    if (curr_time == chkT)
    {
      printf("\n%d\n", cpu_time[curr_process.pid]);
    }
    //Si no ha consumido el time_slice:
    if (cpu_time[curr_process.pid] % time_slice != 0)
    {
      if (curr_process.on_io == 0)  //Si el proceso actual no entró en IO:
      {
        inc_cpu_time(curr_pid);   // Aumentamos su tiempo en CPU
        return curr_pid;          // Y lo seguimos ejecutando.
      }
      else
      {
        //El proceso actual entró en io, por lo que hay que ejecutar el siguiente proceso de mayor prioridad.
        return next_process(procs_info, procs_count, curr_pid, curr_time);
      }      
    }
    else
    {
      //Como ya consumio el time slice:
      //Si la cola no es la de menor prioridad, baja la prioridad del proceso.
      if (active_queue != 0)     
        decrease_priority(curr_pid, curr_process);

      // Ahora ejecuto el siguiente proceso. Ejecutar RR sobre la cola de mayor prioridad.
      return next_process(procs_info, procs_count, curr_pid, curr_time);         
    }
  }

  //Si no hay ningún proceso ejecutándose actualmente:

  //Recorro las colas en orden a ver cuál será la primera con un proceso pendiente:
  for (int i = 2; i >= 0; i--)
  {
    for (int j = 0; j < 200; j++)   
    {
      int c_pid = queues[i][j].pid;
      //hay un proceso o es el proceso 0 que no se ha terminado
      if (c_pid != 0 || queues[i][j].executed_time != 0) 
      {
        //Si el proceso no se ha terminado:
        if (process_ended(procs_info, procs_count, c_pid) == 0)   //O(n)
        {
          //Si el proceso no está en io:
          if (get_io(procs_info, procs_count, c_pid) == 0)
          {
            active_queue = i;     //Se actualiza el valor de la cola activa
            inc_cpu_time(c_pid);  //Aumentar su tiempo en cpu por los próximos 10ms
            return c_pid;   //seguir ejecutándolo        
          }
        }
      }
    }
  }
  //Si llego hasta aqui, solo hay un proceso pendiente
  //printf("ERROR 2\n");
  inc_cpu_time(curr_pid);
  return curr_pid;
  //return 201;
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
