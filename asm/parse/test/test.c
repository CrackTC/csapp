#include "common.h"
#include "parse/node.h"
#include "parse/parser.h"
#include "parse/parsers.h"
#include <stdio.h>

void print_node(parse_node_t *node_ref, int depth) {
  if (node_ref == NULL) {
    return;
  }

  for (int i = 0; i < depth; i++) {
    if (i == depth - 1) {
      printf("├─");
    } else {
      printf("│ ");
    }
  }

  for (const char *p = node_ref->start_ref; p < node_ref->end_ref; p++) {
    putchar(*p);
  }

  putchar('\n');

  for (size_t i = 0; i < node_ref->child_count; i++) {
    print_node(node_ref->children[i], depth + 1);
  }
}

int main() {
  CLEANUP(free_parse_parser_ptr)
  parse_parser_t *dec_digit = new_range_parser('0', '9');
  CLEANUP(free_parse_parser_ptr)
  parse_parser_t *atof = new_range_parser('a', 'f');
  CLEANUP(free_parse_parser_ptr)
  parse_parser_t *AtoF = new_range_parser('A', 'F');
  CLEANUP(free_parse_parser_ptr)
  parse_parser_t *hex_digit = new_one_of_parser(3, dec_digit, atof, AtoF);

  CLEANUP(free_parse_parser_ptr)
  parse_parser_t *hex_digits = new_multiple_parser(hex_digit);
  CLEANUP(free_parse_parser_ptr) parse_parser_t *zero = new_exact_parser('0');
  CLEANUP(free_parse_parser_ptr) parse_parser_t *x = new_exact_parser('x');
  CLEANUP(free_parse_parser_ptr)
  parse_parser_t *hex_number = new_sequence_parser(3, zero, x, hex_digits);

  const char *input = "0x1234567890abcdefABCDEF";
  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(hex_number, input);
  print_node(node, 0);
  return 0;
}
