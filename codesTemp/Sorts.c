
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void swap(void *x, void *y, int size)
{
    char resb[size];
    memcpy(resb, x, size);
    memcpy(x, y, size);
    memcpy(y, resb, size);
}

int compare(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

int getPivot(int li, int ls)
{
    srand((unsigned)time(NULL));
    return rand() % (ls - li + 1) + li;
}

int isSorted(void *arr, int count, size_t size, int (*compare)(const void *, const void *))
{
    for (int i = 0; i < count - 1; i++)
    {
        if (compare(arr + i * size, arr + (i + 1) * size) > 0)
        {
            return 1;
        }
    }
    return 0;
}

void qSort(void *arr, int count, size_t size, int (*compare)(const void *, const void *))
{
    quickSort(arr, 0, count - 1, size, compare);

    if (isSorted(arr, count, size, compare))
    {
        printf("Error in qSort\n");
    }
    return;
}

void quickSort(void *arr, int li, int ls, size_t size, int (*compare)(const void *, const void *))
{
    if (ls <= li || li < 0)
    {
        return;
    }
    printf(" - - - - - - - - - \n");

    swap(arr + ls * size, arr + getPivot(li, ls) * size, size); // pv -> last pos

    int index = li;

    printf("li: %d, ls: %d\n", li, ls + 1);

    for (int i = li; i < ls; i++)
    {
        printf(" %d %d %d\n", *(int *)(arr + i * size), *(int *)(arr + ls * size), compare(arr + i * size, arr + ls * size));
        if (compare(arr + i * size, arr + ls * size) < 0)
        {
            printf("in");
            swap(arr + index * size, arr + i * size, size);
            index++;
        }
    }

    printf("index: %d\n", index);

    swap(arr + index * size, arr + ls * size, size);
    for (int i = 0; i < 10; i++)
    {
        printf(" %d", *(int *)(arr + i * size));
    }
    printf("\n");
    quickSort(arr, li, index - 1, size, compare);
    quickSort(arr, index + 1, ls, size, compare);
    return;
}

int main()
{
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int *bp = a;

    for (int i = 0; i < 10; i++)
    {
        swap(bp + i, bp + getPivot(0, 10), sizeof(int));
    }

    printf("Unordered: ");
    for (int i = 0; i < 10; i++)
    {
        printf(" %d", *(bp + i));
    }

    printf("\n");
    qSort(bp, 10, sizeof(int), compare);
    printf("Ordered: ");

    for (int i = 0; i < 10; i++)
    {
        printf(" %d", *(bp + i));
    }
    printf("\n");
    return 0;
}
