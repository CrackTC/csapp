#ifndef NODE_H
#define NODE_H

typedef struct node_t node_t;

#include "parser.h"
#include <stddef.h>

struct node_t {
  const parser_t *parser_ref;
  const char *start_ref;
  const char *end_ref;
  node_t **children;
  size_t child_count;
};

node_t *new_node(const parser_t *parser, const char *start);
const char *node_add_child(node_t *node, node_t *child);
void free_node(node_t *node);
void free_node_ptr(node_t **node);

#endif // NODE_H
