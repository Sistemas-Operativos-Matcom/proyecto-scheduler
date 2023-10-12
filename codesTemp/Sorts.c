
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void print_arr(int *array, int count)
{
    if (count == 0)
    {
        printf("warning man");
        return;
    }
    for (int i = 0; i < count; i++)
    {
        printf(" %d", array[i]);
    }
    printf("\n");
}

void save(int dest[], int source[])
{
    for(int i = 0 ; i < 5; i++)
    {
        dest[i] = source[i];
    }
    return;
}

int procs[200];
int len;

int main()
{
    int arr[5] = {1, 2, 3, 4, 5};
    save(procs, arr);
    print_arr(procs, 5);
    
}

// typedef struct queue
// {
//     int base;
//     int count;
//     int size;
//     void *arr;
// } queue_t;

// void push_to_queue(queue_t *q, void *item)
// {
//     memcpy(q->arr + q->count * q->size, item, q->size);
//     memcpy(q->count, q->count + 1, q->size);
// }

// void pop_from_queue(queue_t *q, void *item)
// {
//     memcpy(q->base, q->base + 1 * q->size, q->size);
// }

// void *front_queue(queue_t *q, char (*property)(void *item))
// {
//     if (q->count - q->base != 0)
//         printf(property(q->arr + q->base * q->size));
// }

// char property(void *number)
// {
//     char value[1024];
//     sprintf(value, "%d", (int)number);
//     return value;
// }
