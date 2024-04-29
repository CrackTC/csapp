#include "elf.h"
#include "utils.h"
#include <stdlib.h>

elf_t *new_elf(inst_t *text, size_t len) {
  elf_t *result = malloc(sizeof(elf_t));
  result->text = text;
  result->len = len;
  return result;
}

void free_elf(elf_t *elf) { free(elf); }

DEFINE_CLEANUP_FUNC(elf)
