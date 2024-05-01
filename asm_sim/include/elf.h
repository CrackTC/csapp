#pragma once

#include "inst.h"

typedef struct {
  inst_t *text;
} elf_t;

elf_t *new_elf(inst_t *text);
void free_elf(elf_t *elf);
void free_elf_ptr(elf_t **elf);
