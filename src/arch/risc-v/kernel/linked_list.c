#include "linked_list.h"
#include "defs.h"
#include "types.h"

linked_list_node *linked_list_create_node(u64 data) {
  linked_list_node *node = kalloc(sizeof(linked_list_node));
  node->data = data;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

linked_list *linked_list_init() {
  linked_list *list = kalloc(sizeof(linked_list));
  list->size = 0;
  list->first = NULL;
  list->last = NULL;
  return list;
}

void linked_list_destory(linked_list *list) {
  if (list != NULL) {
    kfree(list);
    list = NULL;
  }
}

int linked_list_length(linked_list *list) {
  if (list != NULL)
    return list->size;
  return 0;
}

linked_list_node *linked_list_first(linked_list *list) {
  if (list == NULL || list->size == 0)
    return NULL;
  return list->first;
}

linked_list_node *linked_list_last(linked_list *list) {
  if (list == NULL || list->size == 0)
    return 0;
  return list->last;
}

linked_list_node *linked_list_add_first(linked_list *list,
                                        linked_list_node *e) {
  if (list == NULL)
    return NULL;
  if (list->size == 0) {
    list->first = list->last = e;
  } else {
    e->next = list->first;
    list->first->prev = e;
    list->first = e;
  }
  list->size++;
  return e;
}

linked_list_node *linked_list_add_last(linked_list *list, linked_list_node *e) {
  if (list == NULL || e == NULL)
    return NULL;
  if (list->size == 0) {
    list->first = list->last = e;
  } else {
    e->prev = list->last;
    list->last->next = e;
    list->last = e;
  }
  list->size++;
  return e;
}

linked_list_node *linked_list_remove(linked_list *list, linked_list_node *e) {
  if (list == NULL || list->size == 0 || e == NULL)
    return NULL;
  if (list->size == 1) {
    list->first = list->last = NULL;
  } else if (e == list->first) {
    list->first = e->next;
    list->first->prev = NULL;
  } else if (e == list->last) {
    list->last = e->prev;
    list->last->next = NULL;
  } else {
    e->prev->next = e->next;
    e->next->prev = e->prev;
  }
  e->prev = e->next = NULL;
  list->size--;
  return e;
}

linked_list_node *linked_list_remove_first(linked_list *list) {
  if (list == NULL || list->size == 0)
    return NULL;
  linked_list_node *e = list->first;
  list->first = e->next;
  if (list->first == NULL)
    list->last = NULL;
  else
    list->first->prev = NULL;
  list->size--;
  return e;
}

linked_list_node *linked_list_remove_last(linked_list *list, int pos) {
  if (list == NULL || list->size == 0)
    return NULL;
  linked_list_node *e = list->last;
  list->last = e->prev;
  if (list->last == NULL)
    list->first = NULL;
  else
    list->last->next = NULL;
  list->size--;
  return e;
}