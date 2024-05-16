#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef struct stack_t stack_t;
typedef struct stack_node_t stack_node_t;

stack_t *new_stack();
void stack_add(stack_t *stack, void *data);
void stack_remove(stack_t *stack, stack_node_t *node);
stack_node_t *stack_head(stack_t *stack);
stack_node_t *stack_next(stack_node_t *node);
void *stack_data(stack_node_t *node);
size_t stack_size(stack_t *stack);
void free_stack(stack_t *stack);
void free_stack_ptr(stack_t **stack);

#define STACK_FOR(stack, node)                                                 \
  for (stack_node_t *node = stack_head(stack); node != NULL;                   \
       node = stack_next(node))

#endif // STACK_H
