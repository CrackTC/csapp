#ifndef CPU_H
#define CPU_H

typedef struct core_t core_t;
typedef struct cpu_t cpu_t;

#include "decoder.h"
#include "executor.h"
#include "mem.h"
#include "mmu.h"
#include "reg.h"
#include <stddef.h>

struct core_t {
  reg_t regs;
  flags_t flags;
  decoder_t *decoder;
  executor_t *executor;
};

struct cpu_t {
  core_t **cores;
  size_t core_count;
  mmu_t *mmu;
};

cpu_t *new_cpu(mem_t *mem, size_t core_count);
void free_cpu(cpu_t *cpu);
void free_cpu_ptr(cpu_t **cpu);

#endif // CPU_H
