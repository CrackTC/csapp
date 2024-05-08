#include "cpu.h"
#include "common.h"
#include "decoder.h"
#include "executor.h"
#include "mem.h"
#include "mmu.h"
#include <stddef.h>
#include <stdlib.h>

static core_t *new_core(mmu_t *mmu) {
  core_t *result = malloc(sizeof(core_t));
  result->decoder = new_decoder(&result->regs, mmu);
  result->executor = new_executor(&result->regs, &result->flags, mmu);
  return result;
}

static void free_core(core_t *core) {
  free_executor(core->executor);
  free_decoder(core->decoder);
  free(core);
}

cpu_t *new_cpu(mem_t *mem, size_t core_count) {
  cpu_t *result = malloc(sizeof(cpu_t));
  result->mmu = new_mmu(mem);
  result->cores = malloc(sizeof(core_t *) * core_count);
  for (size_t i = 0; i < core_count; i++) {
    result->cores[i] = new_core(result->mmu);
  }
  return result;
}

void free_cpu(cpu_t *cpu) {
  for (size_t i = 0; i < cpu->core_count; i++) {
    free_core(cpu->cores[i]);
  }
  free(cpu->cores);
  free_mmu(cpu->mmu);
  free(cpu);
}

DEFINE_CLEANUP_FUNC(cpu)
