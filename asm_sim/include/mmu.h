#pragma once

typedef struct mmu_t mmu_t;

#include "machine.h"
#include <stdint.h>

mmu_t *new_mmu(machine_t *machine);
void free_mmu(mmu_t *mmu);
void free_mmu_ptr(mmu_t **mmu);
void *mmu_va2pa(mmu_t *mmu, uint64_t vaddr);
