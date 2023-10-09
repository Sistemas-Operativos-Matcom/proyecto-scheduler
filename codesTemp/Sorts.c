
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int past_procs[205];
int past_procs_len;

int property2(void *item)
{
    int *prop = (int *)item;
    return *prop;
}

void save_procs_info(int dest[], int *source, int *dest_count, int count)
{
    for (int i = 0; i < count; i++)
    {
        dest[i] = source[i];
    }
    *dest_count = count;
}

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

int main()
{
    int arr[5] = {1, 2, 3, 4, 5};
    int count = 5;
    save_procs_info(past_procs, arr, &past_procs_len, 5);
    count = 11;
    arr[2] = 53;
    printf("%d", past_procs_len);
    print_arr(past_procs, past_procs_len);
    //  arr[1] = 3;
    //  count = 3;
    //  save_procs_info(past_procs, arr, past_procs_len, count, sizeof(int), property2);
    //  print_arr(past_procs, *past_procs_len);
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
