#include "mmu.h"
#include "machine.h"
#include "utils.h"
#include <stdlib.h>

struct mmu_t {
  machine_t *machine;
};

mmu_t *new_mmu(machine_t *machine) {
  mmu_t *result = malloc(sizeof(mmu_t));
  result->machine = machine;
  return result;
}

void free_mmu(mmu_t *mmu) { free(mmu); }

DEFINE_CLEANUP_FUNC(mmu)

void *mmu_va2pa(mmu_t *mmu, uint64_t vaddr) {
  return &mmu->machine->mm->space[vaddr % mmu->machine->mm->size];
}
