
#include "queue.h"
#include "simulation.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>




void enqueue(queue_t *queue, int proc_pid) {
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  new_node->pid = proc_pid;
  new_node->next = NULL;

  if (queue->head == NULL) {
    queue->head = new_node;
    queue->tail = new_node;
  } else {
    queue->tail->next = new_node;
    queue->tail = new_node;
  }
}

int dequeue(queue_t *queue) {
  if (queue->head == NULL) {
    int null_process = -1;
    return null_process;
  }

  int proc_pid = queue->head->pid;
  node_t *temp = queue->head;
  queue->head = queue->head->next;
  free(temp);

  if (queue->head == NULL) {
    queue->tail = NULL;
  }

  return proc_pid;
}

int is_empty(queue_t *queue) {
  return queue->head == NULL;
}

