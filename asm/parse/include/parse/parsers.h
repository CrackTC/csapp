#ifndef PARSE_PARSERS_H
#define PARSE_PARSERS_H

#include "parse/parser.h"

parse_parser_t *new_range_parser(char from, char to);
parse_parser_t *new_exact_parser(char ch);
parse_parser_t *new_one_of_parser(size_t n, ...);
parse_parser_t *new_optional_parser(parse_parser_t *single_parser);
parse_parser_t *new_multiple_parser(parse_parser_t *single_parser);
parse_parser_t *new_peek_parser(parse_parser_t *single_parser);
parse_parser_t *new_not_parser(parse_parser_t *single_parser);
parse_parser_t *none_parser();
parse_parser_t *new_sequence_parser(size_t n, ...);
parse_parser_t *new_string_parser(const char *str);
parse_parser_t *new_lowercase_parser(const char *str);

#endif // PARSE_PARSERS_H
