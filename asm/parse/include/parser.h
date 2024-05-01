#ifndef PARSER_H
#define PARSER_H

typedef struct parser_t parser_t;

#include "node.h"

void free_parser_ptr(parser_t **parser);
node_t *parser_parse(const parser_t *parser, const char *input);

#endif // PARSER_H
