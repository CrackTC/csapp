#include "linker/elf_info.h"
#include "linker/linker.h"
#include <stdio.h>
#include <string.h>

void write_elf(elf_t *elf, FILE *stream) {
  /* write elf header */
  fprintf(stream, "ELF: %lu, %lu\n", elf->hdr.lcnt, elf->hdr.shtoff);

  /* write sections */
  size_t current_line = 1;
  for (size_t i = 0; i < elf->seccnt; ++i) {
    section_t *section = &elf->secs[i];

    /* skip .bss, .rel, .symtab section */
    if (strcmp(section->name, ".bss") == 0 ||
        strcmp(section->name, ".rel") == 0 ||
        strcmp(section->name, ".symtab") == 0)
      continue;

    for (size_t j = 0; j < section->size; ++j, ++current_line) {
      fprintf(stream, "%s\n", elf->lines[current_line]);
    }
  }

  /* write rel */
  for (size_t i = 0; i < elf->relcnt; ++i) {
    rel_t *rel = &elf->rels[i];
    fprintf(stream, "0x%lx,0x%lx,0x%lx\n", rel->src_sym_idx, rel->src_off,
            rel->dst_sym_idx);
  }

  /* write symtab */
  for (size_t i = 0; i < elf->symcnt; ++i) {
    sym_t *sym = &elf->syms[i];
    fprintf(stream, "%s,0x%x,0x%x,%hd,0x%lx,0x%lx\n", sym->name, sym->binding,
            sym->type, sym->sec_idx, sym->value, sym->size);
  }

  /* write section table */
  for (size_t i = 0; i < elf->seccnt; ++i) {
    section_t *section = &elf->secs[i];
    fprintf(stream, "%s,0x%lx,0x%lx,0x%lx\n", section->name, section->addr,
            section->off, section->size);
  }
}
