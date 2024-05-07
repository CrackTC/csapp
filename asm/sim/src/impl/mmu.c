#include "mmu.h"
#include "common.h"
#include "mem.h"
#include "stdint.h"
#include <stdlib.h>

struct mmu_t {
  uint8_t *space_ref;
  size_t space_size;
};

mmu_t *new_mmu(mem_t *mem) {
  mmu_t *result = malloc(sizeof(mmu_t));
  result->space_ref = mem->space;
  result->space_size = mem->size;
  return result;
}

void free_mmu(mmu_t *mmu) { free(mmu); }

DEFINE_CLEANUP_FUNC(mmu)

void *mmu_va2pa(mmu_t *mmu, uint64_t vaddr) {
  return &mmu->space_ref[vaddr % mmu->space_size];
}
