#include "elf.h"
#include "common.h"
#include "inst.h"
#include <stdlib.h>

elf_t *new_elf(const inst_t *text, size_t inst_count) {
  elf_t *result = malloc(sizeof(elf_t));
  result->text = text;
  result->inst_count = inst_count;
  return result;
}

void free_elf(elf_t *elf) { free(elf); }

DEFINE_CLEANUP_FUNC(elf)
