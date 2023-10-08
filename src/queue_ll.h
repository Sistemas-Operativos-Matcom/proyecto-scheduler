#ifndef QUEUE_LL_H
#define QUEUE_LL_H
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "simulation.h"

typedef struct Node
{
    int pid;
    struct Node *child;
    int count;
    // struct Nodes *parent;
} tNode;
typedef tNode *root;

root insert_node(root *raiz, int data);
int contains_node(root *raiz, int data);
root remove_node(root *raiz, int data);

#endif
