#include "parse/node.h"
#include "common.h"
#include "parse/parser.h"
#include <stdio.h>
#include <stdlib.h>

parse_node_t *new_parse_node(const parse_parser_t *parser, const char *start) {
  parse_node_t *result = malloc(sizeof(parse_node_t));
  result->parser_ref = parser;
  result->start_ref = start;
  result->end_ref = start;
  result->children = NULL;
  result->child_count = 0;

  return result;
}

const char *node_add_child(parse_node_t *node, parse_node_t *child) {
  if (child == NULL) {
    return NULL;
  }

  ++node->child_count;
  node->children =
      realloc(node->children, node->child_count * sizeof(parse_node_t *));
  node->children[node->child_count - 1] = child;
  node->end_ref = child->end_ref;
  return child->end_ref;
}

void free_parse_node(parse_node_t *node) {
  if (node == NULL) {
    return;
  }

  for (size_t i = 0; i < node->child_count; i++) {
    free_parse_node(node->children[i]);
  }

  free(node->children);
  free(node);
}

DEFINE_CLEANUP_FUNC(parse_node)
