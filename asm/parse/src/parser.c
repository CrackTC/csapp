#include "parser.h"
#include "common.h"
#include "node.h"
#include <stdarg.h>
#include <stdlib.h>

typedef node_t *(*parse_func)(const parser_t *this, const char *input);

struct parser_t {
  parse_func parse;
  void *data;
  parser_t **children_refs;
  size_t child_count;
};

static parser_t *new_parser() {
  parser_t *parser = malloc(sizeof(parser_t));
  parser->children_refs = NULL;
  parser->child_count = 0;
  parser->parse = NULL;
  parser->data = NULL;
  return parser;
}

static void free_parser(parser_t *parser) {
  free(parser->children_refs);
  free(parser->data);
  free(parser);
}

DEFINE_CLEANUP_FUNC(parser)

node_t *parser_parse(const parser_t *parser, const char *input) {
  return parser->parse(parser, input);
}

/* parser definitions */

static node_t *range_parser_parse(const parser_t *this, const char *input) {
  char *data = this->data;
  char from = data[0];
  char to = data[1];
  if (*input >= from && *input <= to) {
    node_t *node = new_node(this, input);
    node->end_ref = input + 1;
    return node;
  }

  return NULL;
}

parser_t *new_range_parser(char from, char to) {
  parser_t *parser = new_parser();
  char *data = malloc(2 * sizeof(char));
  data[0] = from;
  data[1] = to;
  parser->data = data;
  parser->parse = range_parser_parse;
  return parser;
}

static node_t *exact_parser_parse(const parser_t *this, const char *input) {
  char *data = this->data;
  if (*input == *data) {
    node_t *node = new_node(this, input);
    node->end_ref = input + 1;
    return node;
  }

  return NULL;
}

parser_t *new_exact_parser(char ch) {
  parser_t *parser = new_parser();
  char *data = malloc(sizeof(char));
  *data = ch;
  parser->data = data;
  parser->parse = exact_parser_parse;
  return parser;
}

static node_t *one_of_parser_parse(const parser_t *this, const char *input) {
  for (size_t i = 0; i < this->child_count; i++) {
    parser_t *child_parser = this->children_refs[i];
    node_t *result = parser_parse(child_parser, input);
    if (result != NULL) {
      return result;
    }
  }

  return NULL;
}

parser_t *new_one_of_parser(size_t n, ...) {
  parser_t *parser = new_parser();
  parser->child_count = n;
  parser->children_refs = malloc(n * sizeof(parser_t *));

  va_list children;
  va_start(children, n);
  for (size_t i = 0; i < n; i++) {
    parser->children_refs[i] = va_arg(children, parser_t *);
  }
  va_end(children);

  parser->parse = one_of_parser_parse;

  return parser;
}

static node_t *multiple_parser_parse(const parser_t *this, const char *input) {
  parser_t *single_parser = *(parser_t **)this->data;

  node_t *node = new_node(this, input);

  const char *current = input;
  do {
    current = node_add_child(node, parser_parse(single_parser, current));
  } while (current != NULL);

  if (node->child_count != 0) {
    return node;
  }

  free_node(node);
  return NULL;
}

parser_t *new_multiple_parser(const parser_t *single_parser) {
  parser_t *parser = new_parser();
  const parser_t **data = malloc(sizeof(parser_t *));
  *data = single_parser;
  parser->data = data;
  parser->parse = multiple_parser_parse;
  return parser;
}

static node_t *none_parser_parse(const parser_t *this, const char *input) {
  return new_node(this, input);
}

parser_t *new_none_parser() {
  parser_t *parser = new_parser();
  parser->parse = none_parser_parse;
  return parser;
}

static node_t *sequence_parser_parse(const parser_t *this, const char *input) {
  node_t *node = new_node(this, input);
  const char *current = input;
  for (size_t i = 0; i < this->child_count && current != NULL; i++) {
    parser_t *child_parser = this->children_refs[i];
    current = node_add_child(node, parser_parse(child_parser, current));
  }

  if (current != NULL) {
    return node;
  }

  free_node(node);
  return NULL;
}

parser_t *new_sequence_parser(size_t n, ...) {
  parser_t *parser = new_parser();
  parser->child_count = n;
  parser->children_refs = malloc(n * sizeof(parser_t *));

  va_list children;
  va_start(children, n);
  for (size_t i = 0; i < n; i++) {
    parser->children_refs[i] = va_arg(children, parser_t *);
  }
  va_end(children);

  parser->parse = sequence_parser_parse;

  return parser;
}
