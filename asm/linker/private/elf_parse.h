#ifndef ELF_PARSE_H
#define ELF_PARSE_H

#include "linker/elf_info.h"

int parse_elf_header(const char *line, elf_header_t *header);
int parse_section_header(const char *line, section_t *header);
int parse_symbol_entry(const char *line, symbol_t *entry);
int parse_elf(const char **lines, elf_t *info);

#endif // ELF_PARSE_H
