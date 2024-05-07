#ifndef PARSER_H
#define PARSER_H

#include "inst.h"
typedef struct parser_t parser_t;

parser_t *new_parser();
void free_parser(parser_t *parser);
void free_parser_ptr(parser_t **parser);
inst_t *parser_parse_inst(parser_t *parser, const char *line);

#endif // PARSER_H
