#include "mem.h"
#include "common.h"
#include <stdint.h>
#include <stdlib.h>

mem_t *new_mem(size_t size) {
  mem_t *result = malloc(sizeof(mem_t));
  result->size = size;
  result->space = malloc(size * sizeof(uint8_t));
  return result;
}

void free_mem(mem_t *mem) {
  free(mem->space);
  free(mem);
}

DEFINE_CLEANUP_FUNC(mem)
