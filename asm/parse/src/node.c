#include "node.h"
#include "common.h"
#include "parser.h"
#include <stdlib.h>

node_t *new_node(const parser_t *parser, const char *start) {
  node_t *result = malloc(sizeof(node_t));
  result->parser_ref = parser;
  result->start_ref = start;
  result->end_ref = start;
  result->children = NULL;
  result->child_count = 0;

  return result;
}

const char *node_add_child(node_t *node, node_t *child) {
  if (child == NULL) {
    return NULL;
  }

  ++node->child_count;
  node->children =
      realloc(node->children, node->child_count * sizeof(node_t *));
  node->children[node->child_count - 1] = child;
  node->end_ref = child->end_ref;
  return child->end_ref;
}

void free_node(node_t *node) {
  for (size_t i = 0; i < node->child_count; i++) {
    free_node(node->children[i]);
  }

  free(node->children);
  free(node);
}

DEFINE_CLEANUP_FUNC(node)
