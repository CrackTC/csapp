#ifndef PARSE_TRIVIAL_H
#define PARSE_TRIVIAL_H

#include "parse/node.h"
#include "parse/parser.h"
#include <stdint.h>

parse_parser_t *white_space_parser();
parse_parser_t *extend_white_space_parser();
parse_parser_t *white_spaces_parser();
parse_parser_t *delim_parser();
parse_parser_t *hex_number_parser();
parse_parser_t *dec_number_parser();
parse_parser_t *number_parser();
int64_t parse_number(parse_node_t *node);
char *parse_string(parse_node_t *node);

#endif // PARSE_TRIVIAL_H
