#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "simulation.h"
#include "queue_ll.h"

root insert_node(root *raiz, int data)
{
    if (*raiz == NULL)
    {
        root new;
        new = malloc(sizeof(tNode));
        new->count = 1;
        new->pid = data;
        new->child = NULL;
        *raiz = new;
        return new;
    }
    else
    {

        root new;
        new = malloc(sizeof(tNode));
        new->pid = data;
        new->child = *raiz;
        new->count = new->child->count + 1;
        *raiz = new;
        return new;
    }
}
int contains_node(root *raiz, int data)
{
    root aux;
    aux = *raiz;
    while (aux != NULL)
    {
        if (aux->pid == data)
        {
            return 1;
        }
        aux = aux->child;
    }
    return 0;
}
root remove_node(root *raiz, int data)
{
    root newr = *raiz;
    root aux, ant;
    aux = *raiz;
    ant = NULL;
    while (aux != NULL)
    {
        if (aux->pid == data)
        {
            if (ant == NULL)
            {
                *raiz = aux->child;
                newr = aux->child;
                newr->count = aux->count - 1;
            }
            else
            {
                ant->child = aux->child;
                ant->count = aux->count - 1;
            }
            free(aux);
            return;
        }
        ant = aux;
        aux = aux->child;
    }
    return newr;
}
