#include "parse/parser.h"
#include "common.h"
#include "parse/node.h"
#include "parse/parsers.h"
#include "parse/trivial.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef parse_node_t *(*parse_func)(const parse_parser_t *this,
                                    const char *input);

struct parse_parser_t {
  parse_func parse;
  void *data;
  parse_parser_t **children_refs;
  size_t child_count;
  size_t ref_count;
};

static parse_parser_t *new_parse_parser() {
  parse_parser_t *parser = malloc(sizeof(parse_parser_t));
  parser->children_refs = NULL;
  parser->child_count = 0;
  parser->ref_count = 0;
  parser->parse = NULL;
  parser->data = NULL;
  return parser;
}

void free_parse_parser(parse_parser_t *parser) {
  for (size_t i = 0; i < parser->child_count; i++) {
    if (--parser->children_refs[i]->ref_count == 0) {
      free_parse_parser(parser->children_refs[i]);
    }
  }
  free(parser->children_refs);
  free(parser->data);
  free(parser);
}

DEFINE_CLEANUP_FUNC(parse_parser)

parse_node_t *parser_parse(const parse_parser_t *parser, const char *input) {
  return parser->parse(parser, input);
}

/* parser definitions */

static parse_node_t *range_parser_parse(const parse_parser_t *this,
                                        const char *input) {
  char *data = this->data;
  char from = data[0];
  char to = data[1];
  if (*input >= from && *input <= to) {
    parse_node_t *node = new_parse_node(this, input);
    node->end_ref = input + 1;
    return node;
  }

  return NULL;
}

parse_parser_t *new_range_parser(char from, char to) {
  parse_parser_t *parser = new_parse_parser();
  char *data = malloc(2 * sizeof(char));
  data[0] = from;
  data[1] = to;
  parser->data = data;
  parser->parse = range_parser_parse;
  return parser;
}

static parse_node_t *exact_parser_parse(const parse_parser_t *this,
                                        const char *input) {
  char *data = this->data;
  if (*input == *data) {
    parse_node_t *node = new_parse_node(this, input);
    node->end_ref = input + 1;
    return node;
  }

  return NULL;
}

parse_parser_t *new_exact_parser(char ch) {
  parse_parser_t *parser = new_parse_parser();
  char *data = malloc(sizeof(char));
  *data = ch;
  parser->data = data;
  parser->parse = exact_parser_parse;
  return parser;
}

static parse_node_t *one_of_parser_parse(const parse_parser_t *this,
                                         const char *input) {
  for (size_t i = 0; i < this->child_count; i++) {
    parse_parser_t *child_parser = this->children_refs[i];
    parse_node_t *result = parser_parse(child_parser, input);
    if (result != NULL) {
      return result;
    }
  }

  return NULL;
}

parse_parser_t *new_one_of_parser(size_t n, ...) {
  parse_parser_t *parser = new_parse_parser();
  parser->child_count = n;
  parser->children_refs = malloc(n * sizeof(parse_parser_t *));

  va_list children;
  va_start(children, n);
  for (size_t i = 0; i < n; i++) {
    parser->children_refs[i] = va_arg(children, parse_parser_t *);
    parser->children_refs[i]->ref_count++;
  }
  va_end(children);

  parser->parse = one_of_parser_parse;

  return parser;
}

parse_parser_t *new_optional_parser(parse_parser_t *single_parser) {
  return new_one_of_parser(2, single_parser, none_parser());
}

static parse_node_t *multiple_parser_parse(const parse_parser_t *this,
                                           const char *input) {
  parse_parser_t *single_parser = this->children_refs[0];

  parse_node_t *node = new_parse_node(this, input);

  const char *current = input;
  do {
    current = node_add_child(node, parser_parse(single_parser, current));
  } while (current != NULL);

  if (node->child_count != 0) {
    return node;
  }

  free_parse_node(node);
  return NULL;
}

parse_parser_t *new_multiple_parser(parse_parser_t *single_parser) {
  parse_parser_t *parser = new_parse_parser();
  parser->child_count = 1;
  parser->children_refs = malloc(sizeof(parse_parser_t *));
  parser->children_refs[0] = single_parser;
  single_parser->ref_count++;
  parser->parse = multiple_parser_parse;
  return parser;
}

static parse_node_t *peek_parser_parse(const parse_parser_t *this,
                                       const char *input) {
  parse_parser_t *single_parser = this->children_refs[0];

  parse_node_t *child_node = parser_parse(single_parser, input);
  if (child_node == NULL) {
    return NULL;
  }

  free_parse_node(child_node);
  return new_parse_node(this, input);
}

parse_parser_t *new_peek_parser(parse_parser_t *single_parser) {
  parse_parser_t *parser = new_parse_parser();
  parser->child_count = 1;
  parser->children_refs = malloc(sizeof(parse_parser_t *));
  parser->children_refs[0] = single_parser;
  single_parser->ref_count++;
  parser->parse = peek_parser_parse;
  return parser;
}

static parse_node_t *not_parser_parse(const parse_parser_t *this,
                                      const char *input) {
  parse_parser_t *single_parser = this->children_refs[0];

  parse_node_t *child_node = parser_parse(single_parser, input);
  if (child_node == NULL) {
    return new_parse_node(this, input);
  }

  free_parse_node(child_node);
  return NULL;
}

parse_parser_t *new_not_parser(parse_parser_t *single_parser) {
  parse_parser_t *parser = new_parse_parser();
  parser->child_count = 1;
  parser->children_refs = malloc(sizeof(parse_parser_t *));
  parser->children_refs[0] = single_parser;
  single_parser->ref_count++;
  parser->parse = not_parser_parse;
  return parser;
}

static parse_node_t *none_parser_parse(const parse_parser_t *this,
                                       const char *input) {
  return new_parse_node(this, input);
}

parse_parser_t *none_parser() {
  static parse_parser_t *parser = NULL;
  if (parser == NULL) {
    parser = new_parse_parser();
    parser->parse = none_parser_parse;
    parser->ref_count = 1; // never free
  }
  return parser;
}

static parse_node_t *sequence_parser_parse(const parse_parser_t *this,
                                           const char *input) {
  parse_node_t *node = new_parse_node(this, input);
  const char *current = input;
  for (size_t i = 0; i < this->child_count && current != NULL; i++) {
    parse_parser_t *child_parser = this->children_refs[i];
    current = node_add_child(node, parser_parse(child_parser, current));
  }

  if (current != NULL) {
    return node;
  }

  free_parse_node(node);
  return NULL;
}

parse_parser_t *new_sequence_parser(size_t n, ...) {
  parse_parser_t *parser = new_parse_parser();
  parser->child_count = n;
  parser->children_refs = malloc(n * sizeof(parse_parser_t *));

  va_list children;
  va_start(children, n);
  for (size_t i = 0; i < n; i++) {
    parser->children_refs[i] = va_arg(children, parse_parser_t *);
    parser->children_refs[i]->ref_count++;
  }
  va_end(children);

  parser->parse = sequence_parser_parse;

  return parser;
}

static parse_node_t *string_parser_parse(const parse_parser_t *this,
                                         const char *input) {
  const char *data = *(const char **)this->data;
  const char *current = input;
  for (; *data != '\0'; data++, current++) {
    if (*current != *data) {
      return NULL;
    }
  }

  parse_node_t *node = new_parse_node(this, input);
  node->end_ref = current;
  return node;
}

parse_parser_t *new_string_parser(const char *str) {
  parse_parser_t *parser = new_parse_parser();
  const char **data = malloc(sizeof(const char *));
  *data = str;
  parser->data = data;
  parser->parse = string_parser_parse;
  return parser;
}

static parse_node_t *lowercase_parser_parse(const parse_parser_t *this,
                                            const char *input) {
  const char *data = *(const char **)this->data;
  const char *current = input;
  for (; *data != '\0'; data++, current++) {
    if (*current != tolower(*data)) {
      return NULL;
    }
  }

  parse_node_t *node = new_parse_node(this, input);
  node->end_ref = current;
  return node;
}

parse_parser_t *new_lowercase_parser(const char *str) {
  parse_parser_t *parser = new_parse_parser();
  const char **data = malloc(sizeof(const char *));
  *data = str;
  parser->data = data;
  parser->parse = lowercase_parser_parse;
  return parser;
}

/* trivial parsers */

#define DEFINE_TRIVIAL_PARSER(name, expr)                                      \
  parse_parser_t *name##_parser() {                                            \
    static parse_parser_t *parser = NULL;                                      \
    if (parser == NULL) {                                                      \
      parser = expr;                                                           \
      parser->ref_count = 1;                                                   \
    }                                                                          \
    return parser;                                                             \
  }

DEFINE_TRIVIAL_PARSER(white_space, new_one_of_parser(2, new_exact_parser(' '),
                                                     new_exact_parser('\t')));
DEFINE_TRIVIAL_PARSER(extend_white_space,
                      new_one_of_parser(5, new_exact_parser(' '),
                                        new_exact_parser('\t'),
                                        new_exact_parser('\n'),
                                        new_exact_parser('\r'),
                                        new_exact_parser('\0')));
DEFINE_TRIVIAL_PARSER(white_spaces, new_multiple_parser(white_space_parser()));
DEFINE_TRIVIAL_PARSER(
    delim, new_sequence_parser(3, new_optional_parser(white_spaces_parser()),
                               new_exact_parser(','),
                               new_optional_parser(white_spaces_parser())));
DEFINE_TRIVIAL_PARSER(
    hex_number,
    new_sequence_parser(
        3, new_optional_parser(new_exact_parser('-')), new_string_parser("0x"),
        new_multiple_parser(new_one_of_parser(3, new_range_parser('0', '9'),
                                              new_range_parser('a', 'f'),
                                              new_range_parser('A', 'F')))));
DEFINE_TRIVIAL_PARSER(
    dec_number,
    new_sequence_parser(2, new_optional_parser(new_exact_parser('-')),
                        new_multiple_parser(new_range_parser('0', '9'))));
DEFINE_TRIVIAL_PARSER(number, new_one_of_parser(2, hex_number_parser(),
                                                dec_number_parser()));

int64_t parse_number(parse_node_t *node) {
  int64_t result = 0;
  parse_node_t *digits;
  if (node->parser_ref == hex_number_parser()) {
    digits = node->children[2];
    for (const char *c = digits->start_ref; c != digits->end_ref; ++c) {
      result <<= 4;
      if (*c <= '9') {
        result += *c - '0';
      } else if (*c >= 'a') {
        result += *c - 'a' + 10;
      } else {
        result += *c - 'A' + 10;
      }
    }
  } else {
    digits = node->children[1];
    for (const char *c = digits->start_ref; c != digits->end_ref; ++c) {
      result = (result << 3) + (result << 1) + (*c - '0');
    }
  }
  if (node->children[0]->parser_ref != none_parser()) {
    result = -result;
  }
  return result;
}

char *parse_string(parse_node_t *node) {
  return strndup(node->children[0]->start_ref,
                 node->children[0]->end_ref - node->children[0]->start_ref);
}
