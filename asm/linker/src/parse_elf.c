#include "common.h"
#include "linker/elf_info.h"
#include "parse/node.h"
#include "parse/parser.h"
#include "parse/parsers.h"
#include "parse/trivial.h"
#include <stdlib.h>
#include <string.h>

/* ELF: line_count, section_table_start */
static parse_parser_t *elf_header_parser() {
  static parse_parser_t *parser = NULL;

  if (parser != NULL) {
    return parser;
  }

  parse_parser_t *line_count = new_sequence_parser(
      2, new_optional_parser(white_spaces_parser()), number_parser());

  parse_parser_t *section_table_start =
      new_sequence_parser(2, delim_parser(), number_parser());

  parser = new_sequence_parser(3, new_string_parser("ELF:"), line_count,
                               section_table_start);

  return parser;
}

int parse_elf_header(const char *line, elf_header_t *header) {
  *header = (elf_header_t){0};
  parse_parser_t *parser = elf_header_parser(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  header->line_count = parse_number(node->children[1]->children[1]);
  header->section_table_start = parse_number(node->children[2]->children[1]);
  return 0;
}

/* name, addr, offset, size */
static parse_parser_t *section_header_parser() {
  static parse_parser_t *parser = NULL;

  if (parser != NULL) {
    return parser;
  }

  parse_parser_t *name = new_sequence_parser(
      2, new_optional_parser(white_spaces_parser()),
      new_multiple_parser(new_not_parser(new_one_of_parser(
          2, new_exact_parser(','), extend_white_space_parser()))));

  parse_parser_t *addr =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *offset =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *size =
      new_sequence_parser(2, delim_parser(), number_parser());

  parser = new_sequence_parser(4, name, addr, offset, size);

  return parser;
}

int parse_section_header(const char *line, section_header_t *header) {
  *header = (section_header_t){0};
  parse_parser_t *parser = section_header_parser(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  header->name = parse_string(node->children[0]->children[1]);
  header->address = parse_number(node->children[1]->children[1]);
  header->offset = parse_number(node->children[2]->children[1]);
  header->size = parse_number(node->children[3]->children[1]);
  return 0;
}

/* name, bind, type, section, offset, size */
static parse_parser_t *symbol_entry_parser() {
  static parse_parser_t *parser = NULL;

  if (parser != NULL) {
    return parser;
  }

  parse_parser_t *name = new_sequence_parser(
      2, new_optional_parser(white_spaces_parser()),
      new_multiple_parser(new_not_parser(new_one_of_parser(
          2, new_exact_parser(','), extend_white_space_parser()))));

  parse_parser_t *bind =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *type =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *section =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *offset =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *size =
      new_sequence_parser(2, delim_parser(), number_parser());

  parser = new_sequence_parser(6, name, bind, type, section, offset, size);

  return parser;
}

int parse_symbol_entry(const char *line, symbol_t *entry) {
  *entry = (symbol_t){0};
  parse_parser_t *parser = symbol_entry_parser(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  entry->name = parse_string(node->children[0]->children[1]);
  entry->binding = parse_number(node->children[1]->children[1]);
  entry->type = parse_number(node->children[2]->children[1]);
  entry->section = parse_number(node->children[3]->children[1]);
  entry->value = parse_number(node->children[4]->children[1]);
  entry->size = parse_number(node->children[5]->children[1]);
  return 0;
}

int parse_elf(const char **lines, elf_info_t *info) {
  *info = (elf_info_t){0};
  if (parse_elf_header(lines[0], &info->header) != 0) {
    return -1;
  }

  size_t section_count =
      info->header.line_count - info->header.section_table_start;
  info->section_headers = malloc(sizeof(section_header_t) * section_count);

  size_t i;
  for (i = info->header.section_table_start; i < info->header.line_count; ++i) {
    if (parse_section_header(lines[i], &info->section_headers[i]) != 0) {
      return -1;
    }
  }

  // find the symbol table
  for (i = 0; i < section_count; ++i) {
    if (strcmp(info->section_headers[i].name, ".symtab") == 0) {
      info->symbol_count = info->section_headers[i].size;
      info->symbols = malloc(sizeof(symbol_t) * info->symbol_count);
      for (size_t j = 0; j < info->symbol_count; ++j) {
        if (parse_symbol_entry(lines[info->section_headers[i].offset + j],
                               &info->symbols[j]) != 0) {
          return -1;
        }
      }
      break;
    }
  }

  if (i == section_count) {
    return -1;
  }

  return 0;
}