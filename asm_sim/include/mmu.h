#ifndef MMU_H
#define MMU_H

typedef struct mmu_t mmu_t;

#include "mem.h"
#include <stdint.h>

mmu_t *new_mmu(mem_t *mem);
void free_mmu(mmu_t *mmu);
void free_mmu_ptr(mmu_t **mmu);
void *mmu_va2pa(mmu_t *mmu, uint64_t vaddr);

#endif // MMU_H
