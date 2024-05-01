#ifndef ELF_H
#define ELF_H

#include "inst.h"

typedef struct {
  const inst_t *text;
} elf_t;

elf_t *new_elf(const inst_t *text);
void free_elf(elf_t *elf);
void free_elf_ptr(elf_t **elf);

#endif // ELF_H
