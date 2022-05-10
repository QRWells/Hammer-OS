#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include "types.h"

typedef struct linked_list_node_struct {
  struct linked_list_node_struct *prev;
  struct linked_list_node_struct *next;
  u64 data;
} linked_list_node;

linked_list_node *linked_list_create_node(u64 data);

typedef struct linked_list_struct {
  int size;
  linked_list_node *first;
  linked_list_node *last;
} linked_list;

linked_list *linked_list_init();
void linked_list_destory(linked_list *list);
int linked_list_length(linked_list *list);
linked_list_node *linked_list_first(linked_list *list);
linked_list_node *linked_list_last(linked_list *list);
linked_list_node *linked_list_add_first(linked_list *list, linked_list_node *e);
linked_list_node *linked_list_add_last(linked_list *list, linked_list_node *e);
linked_list_node *linked_list_remove(linked_list *list, linked_list_node *e);
linked_list_node *linked_list_remove_first(linked_list *list);
linked_list_node *linked_list_remove_last(linked_list *list, int pos);

#endif // _LINKED_LIST_H_