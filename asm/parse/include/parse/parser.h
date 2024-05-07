#ifndef PARSE_PARSER_H
#define PARSE_PARSER_H

typedef struct parse_parser_t parse_parser_t;

#include "parse/node.h"

void free_parse_parser(parse_parser_t *parser);
void free_parse_parser_ptr(parse_parser_t **parser);
parse_node_t *parser_parse(const parse_parser_t *parser, const char *input);

#endif // PARSE_PARSER_H
