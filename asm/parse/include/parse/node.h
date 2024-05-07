#ifndef PARSE_NODE_H
#define PARSE_NODE_H

typedef struct parse_node_t parse_node_t;

#include "parse/parser.h"
#include <stddef.h>

struct parse_node_t {
  const parse_parser_t *parser_ref;
  const char *start_ref;
  const char *end_ref;
  parse_node_t **children;
  size_t child_count;
};

parse_node_t *new_parse_node(const parse_parser_t *parser, const char *start);
const char *node_add_child(parse_node_t *node, parse_node_t *child);
void free_parse_node(parse_node_t *node);
void free_parse_node_ptr(parse_node_t **node);

#endif // PARSE_NODE_H
