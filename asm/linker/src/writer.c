#include "linker/elf_info.h"
#include "linker/linker.h"
#include <stdio.h>

void write_elf(elf_t *elf, FILE *stream) {
  /* write elf header */
  fprintf(stream, "ELF: %lu %lu\n", elf->header.line_count,
          elf->header.section_table_start);

  /* write sections */
  size_t current_line = 1;
  for (size_t i = 0; i < elf->section_count - 1; ++i) {
    section_t *section = &elf->sections[i];
    for (size_t j = 0; j < section->size; ++j, ++current_line) {
      fprintf(stream, "%s\n", elf->lines[current_line]);
    }
  }

  /* write symtab */
  for (size_t i = 0; i < elf->symbol_count; ++i) {
    symbol_t *sym = &elf->symbols[i];
    fprintf(stream, "%s,%u,%u,%hd,%lu,%lu\n", sym->name, sym->binding,
            sym->type, sym->section, sym->value, sym->size);
  }

  for (size_t i = 0; i < elf->section_count; ++i) {
    section_t *section = &elf->sections[i];
    fprintf(stream, "%s,%lu,%lu,%lu\n", section->name, section->address,
            section->offset, section->size);
  }
}
