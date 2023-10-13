
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

void merge_update(int past_pid[], int *current_procs, int *past_count, int procs_count)
{
    int ipp = 0;
    int icp = 0;
    int icopy = 0;

    while (ipp < (*past_count) && icp < procs_count)
    {
        
        if (past_pid[ipp] == current_procs[icp])
        {
            past_pid[icopy] = past_pid[ipp];
            level_pid[icopy] = past_pid[icopy];
            time_pid[icopy] = past_pid[icopy];
            icp++;
            icopy++;
        }
        ipp++;
    }

    // annadir  los nuevos
    while (icopy < procs_count)
    {
        past_pid[icopy] = current_procs[icp];
        level_pid[icopy] = 0;
        time_pid[icopy] = 0;
        icopy++;
        icp++;
    }
    *past_count = procs_count;
    return;
}

int procs[200];
// int lv[200];
// int timeppd[200];
int len = 8;

int main()
{
    printf("hello world");
    int array[10] = {1, 2, 3, 4, 5, 7, 8, 2};
    int *arr = array;
    // merge_update(procs, lv, timeppd, arr, &len, 5);

    merge_update(procs, arr, &len, 8);
    printf("%d  ", len);
    print_arr(procs, len);
    printf("  ");

    int perra[10] = {1, 2, 34, 4, 5, 7, 8, 2};
    int *perrap = perra;

    merge_update(procs, perrap, &len, 8);
    printf("%d  ", len);
    print_arr(procs, len);

    printf("  ");
}