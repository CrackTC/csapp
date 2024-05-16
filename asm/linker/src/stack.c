#include "stack.h"
#include "common.h"
#include <stdlib.h>

struct stack_t {
  stack_node_t *head;
  size_t size;
};

struct stack_node_t {
  void *data;
  stack_node_t *next;
};

stack_t *new_stack() {
  stack_t *stack = malloc(sizeof(stack_t));
  *stack = (stack_t){0};
  return stack;
}

void stack_add(stack_t *stack, void *data) {
  stack_node_t *node = malloc(sizeof(stack_node_t));
  *node = (stack_node_t){.data = data, .next = stack->head};
  stack->head = node;
  ++stack->size;
}

void stack_remove(stack_t *stack, stack_node_t *node) {
  if (stack->head == node) {
    stack->head = node->next;
    --stack->size;
    free(node);
    return;
  }

  stack_node_t *prev = stack->head;
  while (prev->next != node) {
    prev = prev->next;
  }

  prev->next = node->next;
  --stack->size;
  free(node);
}

stack_node_t *stack_head(stack_t *stack) { return stack->head; }

stack_node_t *stack_next(stack_node_t *node) { return node->next; }

void *stack_data(stack_node_t *node) { return node->data; }
size_t stack_size(stack_t *stack) { return stack->size; }

void free_stack(stack_t *stack) {
  stack_node_t *node = stack->head;
  while (node != NULL) {
    stack_node_t *next = node->next;
    free(node);
    node = next;
  }
  free(stack);
}

DEFINE_CLEANUP_FUNC(stack)
