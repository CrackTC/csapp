#include "list.h"
#include "common.h"
#include <stdlib.h>

struct list_t {
  list_node_t *head;
  size_t size;
};

struct list_node_t {
  void *data;
  list_node_t *next;
};

list_t *new_list() {
  list_t *list = malloc(sizeof(list_t));
  *list = (list_t){0};
  return list;
}

void list_add(list_t *list, void *data) {
  list_node_t *node = malloc(sizeof(list_node_t));
  *node = (list_node_t){.data = data, .next = list->head};
  list->head = node;
  ++list->size;
}

void list_remove(list_t *list, list_node_t *node) {
  if (list->head == node) {
    list->head = node->next;
    --list->size;
    free(node);
    return;
  }

  list_node_t *prev = list->head;
  while (prev->next != node) {
    prev = prev->next;
  }

  prev->next = node->next;
  --list->size;
  free(node);
}

list_node_t *list_head(list_t *list) { return list->head; }

list_node_t *list_next(list_node_t *node) { return node->next; }

void *list_data(list_node_t *node) { return node->data; }
size_t list_size(list_t *list) { return list->size; }

void free_list(list_t *list) {
  list_node_t *node = list->head;
  while (node != NULL) {
    list_node_t *next = node->next;
    free(node);
    node = next;
  }
  free(list);
}

DEFINE_CLEANUP_FUNC(list)
