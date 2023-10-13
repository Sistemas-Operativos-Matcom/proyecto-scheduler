## Initial Workload:
- los procesos no hacen i/o.
- todos llegan en el mismo momento.
- se conoce el tiempo que demora each.
- todos demoran la misma cantidad de tiempo.

## Scheduling:

```c
int scheduler_name(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
    // ...
}
```

La interfaz que me es dada está compuesta por:

- un array de procesos, que serían los dos primeros argumentos, lo que contiene ese array son procesos que han arrived al sistema y no han finalizado yet, so podrían estar esperando por i/o, inactivos en modo starving, o ejecutándose actualmente.
- el segundo argumento es el tiempo que ha trnascurrido desde que la simuación comenzó.
- el tercer argumento es el *curr_pid* de el proceso que se está ejecutando actualmente, so digamos que mi simulación comienza con algún proceso, si ese proceso por casualidades del destino acaba antes de hacerse el time interrupt, el *curr_pid* que me llegaría como argumento es $-1$, so eso debe ser handled.

La función debe debolb

Más alla de los argumentos de esa función:

-  el `struct proc_info_t` posee:
	- el tiempo en que se ha ejecutado ese proceso específico. ( que no es handled by myself, sino por la simulación ).
	- si un proceso está en i/o en ese momento específico, que es un `bool`.
- existe una función `process_total_time` que me devuelve el tiempo que durará un proceso en específico notar que esto es una de las asumptions.
- el time interrupt se realiza cada 10 ms, eso significa que en la simulación mi función `scheduler` va a ser llamada cada 10 ms.

El scheduler debe devolver:

- $-1$: No se ejecuta ningún proceso.	
- curr_pid: Se mantiene en ejecución el proceso actual.
- PID diferente al curr_pid: Simula un cambio de contexto y se ejecuta el proceso indicado.

## Implementación Details:

Por ahora la forma de inicializar las estructuras de datos va a ser declarar un puntero a ellas global, para que el scheduler implementado por mí pueda acceder a él, e inicializar ese punto dentro de la función que selecciona que scheduler usar, para que esté ready para ser usada por mi scheduler. ( Looking for something better ).

Lo más trivial para implementar es usar un array y definir sobre él todas las operaciones que esté interesado hacer. Eso sería fácil de implementar lo que es $O(n)$ realizar todas las operaciones sobre él.

It seems que independiente de que método choose el algoritmo es:
	- actualiza el estado del último que mande a ejecutar.
	- añade los nuevos que arrived.
	- decide cual es el next a ejecutar.

## Consistency Check:

Siempre que es llamada la función recibo un array de *proc_info* que serían todos los procesos que están activos.

- notar que en *proc_info* pueden haber procesos que no están en mi *DA*, eso means que ellos arrived en algún momento después del último Time Interrupt, por lo que debo añadirlos a mi DA.
- lo otro es que el proceso que mandé a ejecutar finalizó por lo que debo quitarlo de mi *DA*, 
- otra cosa weird que pudo haber pasado es que el proceso que mandé a ejecutar entró en i/o state, en el meantime, en particular solamente tendría que actualizar la información de ese proceso en mi DA, y que después ella se encarge de que hacer.

Mi DA debe guardar el PID del último proceso que mandó a ejecutar, para que así pueda decidir que hacer analizando el PID, actualizar su info, o borrarlo de mi DA, entonces como no tengo idea de como son managed los procesos que están en el array *process_info*,  For sanity reasons voy a hacer una copia de los procesos y actualizar los valores de el último que mandé a ejecutar.


### Scheduling Estrategias:

#### First In First Out:

Mandar a ejecutar a los procesos en orden de llegada. 
