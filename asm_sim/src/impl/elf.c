#include "elf.h"
#include "inst.h"
#include "utils.h"
#include <stdlib.h>

elf_t *new_elf(const inst_t *text) {
  elf_t *result = malloc(sizeof(elf_t));
  result->text = text;
  return result;
}

void free_elf(elf_t *elf) { free(elf); }

DEFINE_CLEANUP_FUNC(elf)
