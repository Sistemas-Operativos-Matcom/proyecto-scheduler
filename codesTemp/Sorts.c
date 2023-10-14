
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

void save(int dest[], int *source)
{
    for (int i = 0; i < 5; i++)
    {
        dest[i] = source[i];
        // printf("%d", dest[i]);
    }
    return;
}
void testing(int *x, int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%d ", x[i]);
    }
}

void pretesting(int *x, int *count)
{
    testing(x, *count);
}


int main()
{
    int a[5] = {1, 2655, 332, 4, 5};
    int b = 5;
    pretesting(a, &b);
}
