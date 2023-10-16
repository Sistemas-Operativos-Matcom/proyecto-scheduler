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

- un array de procesos, que serían los dos primeros argumentos, lo que contiene ese array son procesos que han arrived al sistema y no han finalizado yet, so podrían estar en:
	- modo *I/O*
	- inactivos en modo starving
	- ejecutándose actualmente.

- el segundo argumento es el tiempo que ha trnascurrido desde que la simulación comenzó.

- el tercer argumento es el *curr_pid* de el último proceso que se ejecutó en el CPU, si llega $-1$ significa que en este último time slice no hubo nada en el CPU, por otro lado si me llega un *curr_pid* válido debo tener en cuenta que ese proceso puede estar ahora en i/o, so no tendría sentido volver a enviarlo a el CPU si hay algún otro proceso que esté activo y no en i/o.

Más alla de los argumentos de esa función:

-  el `struct proc_info_t` posee:
	- el tiempo en que se ha ejecutado ese proceso específico. ( que no es handled by myself, sino por la simulación ).
	- si un proceso está en i/o en ese momento específico, que es un `bool`.
- existe una función `process_total_time` que me devuelve el tiempo que durará un proceso en específico notar que esto es una de las asumptions.
- el time interrupt se realiza cada 10 ms, eso significa que en la simulación mi función `scheduler` va a ser llamada cada 10 ms.
otro.

El scheduler debe devolver:

- $-1$: No se ejecuta ningún proceso.	
- curr_pid: se vuelve a ejecutar el último proceso que estuvo en el CPU, por lo que puede que no sea necesario hacer un context switch.
- PID diferente al *curr_pid* (entre los PID que están en el array): Simula un cambio de contexto y se ejecuta el proceso indicado.

## Implementación Details:

En mi $DS$ always los procesos que tengo tienen que ser un subset de los procesos que están en el array de procesos. Eso tiene que ser chequeado inicialmente por consistencia. That allows only  necesario guardar el *pid* del proceso y en el caso de *MLFQ* el slice de ese proceso, cualquier otra información relevante se puede obtener del array de procesos que le es pasado al scheduler. So por eso uso una función que se llama *sync_ds*, esto puede resultar innecesario dado que el array de procesos dado contiene los procesos en orden de llegada, pero permitiría que mi scheduling funcione independientemente de el orden que poseen los procesos en ese array ( le puedo dar yo el orden que quiera).

Hasta ahora es super lento porque cada vez que me es llamada la función es realizado un *sync_ds*, so no optimizations by now.

## Resultados