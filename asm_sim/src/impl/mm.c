#include "mm.h"
#include "utils.h"
#include <stdlib.h>

mm_t *new_mm(size_t size) {
  mm_t *result = malloc(sizeof(mm_t));
  result->size = size;
  result->space = malloc(size * sizeof(uint8_t));
  return result;
}

void free_mm(mm_t *mm) {
  free(mm->space);
  free(mm);
}

DEFINE_CLEANUP_FUNC(mm)
