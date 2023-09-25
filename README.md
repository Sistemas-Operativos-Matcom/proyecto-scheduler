# Scheduler

Implementa las diferentes estrategias de schedulers estudiados en clase y
evalúa las ventajas y desventajas de cada una según las métricas vistas en
conferencia.

Debes implementar cada estrategia en el archivo `src/schedulers.c`. En el mismo
se encuentra una implementación de un scheduler tipo FIFO. Además podrás
encontrar las instrucciones para la creación de otras estrategias.

## Compilar y ejecutar el proyecto

Para compilar el proyecto puedes usar:

```bash
./build.sh
```

El ejecutable lo encontrarás en `./build/main` una vez compile el proyecto.

Los scripts `test_single.sh` y `test_all.sh` prueban un scheduler con uno o
todos los casos de prueba respectivamente. Ambos scripts compilan el proyecto
automáticamente. Puedes usarlos de la siguiente forma:

```bash
./test_single.sh CASO-DE-PRUEBA NOMBRE-DEL-SCHEDULER [OPCIONES]
./test_all.sh CARPETA NOMBRE-DEL-SCHEDULER [OPCIONES]
```

> Los parámetros necesarios para ejecutar directaemte el proyecto son los
> mismos que se usan para ejecutar el script `test_single.sh`.

Por ejemplo:

```bash
./test_single.sh ./test_cases/case_001.txt fifo
./test_all.sh ./test_cases fifo
```

Las opciones se indican mediante una serie de caracteres (sin espacio
intermedio). Las opciones disponibles son:

- `g`: Muestra la gráfica de ejecución.
- `r`: Ejecuta la simulación en tiempo real.

Por ejemplo:

```bash
./test_single.sh ./test_cases/case_01.txt fifo rg
./test_single.sh ./test_cases/case_01.txt fifo g
```

Leyenda para la gráfica de ejecución:

- `.`: El proceso no está en el sistema
- `=`: El proceso está en el sistema pero no está realizando ninguna operación
- `O`: El proceso está ejecutándose en el CPU
- `I`: El proceso está realizando una operación I/O

## Casos de prueba

La información de la ejecución de un proceso está codificada en dos partes: el
tiempo de llegada del proceso y las interrupciones que hace el mismo entre CPU
e I/O. Por ejemplo:

```
15 30 40 20
```

Esto representa un proceso que arriva en el ms 15, se ejecuta por 30ms en el
CPU, luego hace una interrupción I/O que demora 40ms y finalmente vuelve a
ejecutarse en el CPU por 20ms.

Cada línea de cada caso de prueba contiene la información de la ejecución de un
proceso. Se asegura además que los procesos en un caso de prueba están
ordenados por su tiempo de llegada.
