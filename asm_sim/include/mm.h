#pragma once

typedef struct mm_t mm_t;

#include <stddef.h>
#include <stdint.h>

struct mm_t {
  uint8_t *space;
  size_t size;
};

mm_t *new_mm(size_t size);
void free_mm(mm_t *mm);
void free_mm_ptr(mm_t **mm);
