#pragma once

typedef struct cpu_t cpu_t;

#include "decoder.h"
#include "executor.h"
#include "machine.h"
#include "mmu.h"
#include "reg.h"

struct cpu_t {
  reg_t regs;
  decoder_t *decoder;
  executor_t *executor;
  mmu_t *mmu;
};

cpu_t *new_cpu(machine_t *machine);
void free_cpu(cpu_t *cpu);
void free_cpu_ptr(cpu_t **cpu);
