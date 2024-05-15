#include "common.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  CLEANUP(free_list_ptr) list_t *list = new_list();

  /* insertion */
  for (int i = 0; i < 10; i++) {
    int *data = malloc(sizeof(int));
    *data = i;
    list_add(list, data);
  }

  list_node_t *node = list_head(list);
  int i = 9;
  while (node != NULL) {
    assert(*(int *)list_data(node) == i);
    node = list_next(node);
    --i;
  }

  assert(i == -1);

  fputs("insertion test passed\n", stderr);

  /* deletion */
  node = list_head(list);
  while (node != NULL) {
    int data = *(int *)list_data(node);
    if (data % 2 != 0) {
      fprintf(stderr, "deleting %d\n", data);
      list_node_t *next = list_next(node);
      free(list_data(node));
      list_remove(list, node);
      node = next;
    } else {
      node = list_next(node);
    }
  }

  node = list_head(list);
  i = 8;
  while (node != NULL) {
    assert(*(int *)list_data(node) == i);
    free(list_data(node));
    node = list_next(node);
    i -= 2;
  }

  assert(i == -2);
  fputs("deletion test passed\n", stderr);

  return 0;
}
