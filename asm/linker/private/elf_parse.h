#ifndef ELF_PARSE_H
#define ELF_PARSE_H

#include "linker/elf_info.h"

int parse_elf_hdr(const char *line, elf_header_t *header);
int parse_sec_hdr(const char *line, section_t *header);
int parse_sym(const char *line, sym_t *entry);
int parse_rel(const char *line, rel_t *entry);
int parse_elf(const char **lines, elf_t *info);

#endif // ELF_PARSE_H
