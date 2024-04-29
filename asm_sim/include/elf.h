#pragma once

#include "inst.h"
#include <stddef.h>

typedef struct {
  inst_t *text;
  size_t len;
} elf_t;

elf_t *new_elf(inst_t *text, size_t len);
void free_elf(elf_t *elf);
void free_elf_ptr(elf_t **elf);
