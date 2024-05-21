#include "common.h"
#include "elf_parse.h"
#include "linker/elf_info.h"
#include "parse/node.h"
#include "parse/parser.h"
#include "parse/parsers.h"
#include "parse/trivial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ELF: line_count, section_table_start */
static parse_parser_t *elf_hdr() {
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

int parse_elf_hdr(const char *line, elf_header_t *header) {
  *header = (elf_header_t){0};
  puts(line);
  parse_parser_t *parser = elf_hdr(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  header->lcnt = parse_number(node->children[1]->children[1]);
  header->shtoff = parse_number(node->children[2]->children[1]);
  return 0;
}

/* name, addr, offset, size */
static parse_parser_t *sec_hdr() {
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

int parse_sec_hdr(const char *line, section_t *header) {
  puts(line);
  *header = (section_t){0};
  parse_parser_t *parser = sec_hdr(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  header->name = parse_string(node->children[0]->children[1]);
  header->addr = parse_number(node->children[1]->children[1]);
  header->off = parse_number(node->children[2]->children[1]);
  header->size = parse_number(node->children[3]->children[1]);
  return 0;
}

/* name, bind, type, section, offset, size */
static parse_parser_t *sym() {
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

int parse_sym(const char *line, sym_t *entry) {
  puts(line);
  *entry = (sym_t){0};
  parse_parser_t *parser = sym(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  entry->name = parse_string(node->children[0]->children[1]);
  entry->binding = parse_number(node->children[1]->children[1]);
  entry->type = parse_number(node->children[2]->children[1]);
  entry->sec_idx = parse_number(node->children[3]->children[1]);
  entry->value = parse_number(node->children[4]->children[1]);
  entry->size = parse_number(node->children[5]->children[1]);
  return 0;
}

static parse_parser_t *rel() {
  static parse_parser_t *parser = NULL;

  if (parser != NULL) {
    return parser;
  }

  parse_parser_t *src_sym_index = new_sequence_parser(
      2, new_optional_parser(white_spaces_parser()), number_parser());

  parse_parser_t *src_offset =
      new_sequence_parser(2, delim_parser(), number_parser());

  parse_parser_t *dst_sym_index =
      new_sequence_parser(2, delim_parser(), number_parser());

  parser = new_sequence_parser(3, src_sym_index, src_offset, dst_sym_index);

  return parser;
}

int parse_rel(const char *line, rel_t *entry) {
  puts(line);
  *entry = (rel_t){0};
  parse_parser_t *parser = rel(); /* no need to free */

  CLEANUP(free_parse_node_ptr)
  parse_node_t *node = parser_parse(parser, line);
  if (node == NULL) {
    return -1;
  }

  entry->src_sym_idx = parse_number(node->children[0]->children[1]);
  entry->src_off = parse_number(node->children[1]->children[1]);
  entry->dst_sym_idx = parse_number(node->children[2]->children[1]);
  return 0;
}

int parse_elf(const char **lines, elf_t *elf) {
  *elf = (elf_t){0};

  if (parse_elf_hdr(lines[0], &elf->hdr) != 0) {
    fprintf(stderr, "Failed to parse ELF header\n");
    return -1;
  }

  elf->lines = malloc(sizeof(char *) * elf->hdr.lcnt);
  for (size_t l = 0; l < elf->hdr.lcnt; ++l) {
    elf->lines[l] = strdup(lines[l]);
  }

  uint64_t section_count = elf->hdr.lcnt - elf->hdr.shtoff;
  elf->seccnt = section_count;
  elf->secs = malloc(sizeof(section_t) * section_count);

  size_t i;
  for (i = 0; i < section_count; ++i) {
    if (parse_sec_hdr(lines[elf->hdr.shtoff + i], &elf->secs[i]) != 0) {
      fprintf(stderr, "Failed to parse section header\n");
      return -1;
    }
  }

  for (i = 0; i < section_count; ++i) {
    puts(elf->secs[i].name);
    // find the symbol table
    if (strcmp(elf->secs[i].name, ".symtab") == 0) {
      elf->symcnt = elf->secs[i].size;
      elf->syms = malloc(sizeof(sym_t) * elf->symcnt);
      for (size_t j = 0; j < elf->symcnt; ++j) {
        if (parse_sym(lines[elf->secs[i].off + j], &elf->syms[j]) != 0) {
          fprintf(stderr, "Failed to parse symbol entry\n");
          return -1;
        }
      }
      break;
    } else if (strcmp(elf->secs[i].name, ".rel") == 0) {
      elf->relcnt = elf->secs[i].size;
      elf->rels = malloc(sizeof(rel_t) * elf->relcnt);
      for (size_t j = 0; j < elf->relcnt; ++j) {
        if (parse_rel(lines[elf->secs[i].off + j], &elf->rels[j]) != 0) {
          fprintf(stderr, "Failed to parse relocation entry\n");
          return -1;
        }
      }
    }
  }

  if (i == section_count) {
    fprintf(stderr, "No symbol table found\n");
    return -1;
  }

  return 0;
}
