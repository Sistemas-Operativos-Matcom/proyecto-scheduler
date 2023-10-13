#ifndef AUXILIAR
#define AUXILIAR

#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

void q_sort(void *arr, int count, size_t size, int (*compare)(const void *, const void *));

#endif