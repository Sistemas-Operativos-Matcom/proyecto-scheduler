#ifndef QUEUE_H
#define QUEUE_H

#include "simulation.h"
#include <stdbool.h>

  typedef struct node{
    proc_info_t value;
    struct node* next;
  }node_t;

  typedef struct queue{
    node_t* first;
    node_t* last;
    int len;
    node_t* nxt;
  }queue_t;

  void start_queue(queue_t* q);
  void append_queue(queue_t* q, proc_info_t p);
  void remove_queue(queue_t* q, int curr_pid);
  bool remove_node(node_t* n, node_t* end, int curr_pid);
  bool find_queue(queue_t* q, proc_info_t p);
  bool find_node(node_t* n, node_t* end, proc_info_t p);

#endif