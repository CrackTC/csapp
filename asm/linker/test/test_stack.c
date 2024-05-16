#include "common.h"
#include "stack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  CLEANUP(free_stack_ptr) stack_t *stack = new_stack();

  /* insertion */
  for (int i = 0; i < 10; i++) {
    int *data = malloc(sizeof(int));
    *data = i;
    stack_add(stack, data);
  }

  assert(stack_size(stack) == 10);

  stack_node_t *node = stack_head(stack);
  int i = 9;
  while (node != NULL) {
    assert(*(int *)stack_data(node) == i);
    node = stack_next(node);
    --i;
  }

  assert(i == -1);

  fputs("insertion test passed\n", stderr);

  /* deletion */
  node = stack_head(stack);
  while (node != NULL) {
    int data = *(int *)stack_data(node);
    if (data % 2 != 0) {
      fprintf(stderr, "deleting %d\n", data);
      stack_node_t *next = stack_next(node);
      free(stack_data(node));
      stack_remove(stack, node);
      node = next;
    } else {
      node = stack_next(node);
    }
  }

  assert(stack_size(stack) == 5);

  node = stack_head(stack);
  i = 8;
  while (node != NULL) {
    assert(*(int *)stack_data(node) == i);
    free(stack_data(node));
    node = stack_next(node);
    i -= 2;
  }

  assert(i == -2);
  fputs("deletion test passed\n", stderr);

  return 0;
}
