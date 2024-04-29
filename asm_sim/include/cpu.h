#pragma once

typedef struct cpu_t cpu_t;

#include "decoder.h"
#include "executor.h"
#include "machine.h"
#include "mmu.h"
#include "reg.h"

struct cpu_t {
  struct {
    QWORD_REG_ALPHA(a);
    QWORD_REG_ALPHA(b);
    QWORD_REG_ALPHA(c);
    QWORD_REG_ALPHA(d);

    QWORD_REG_SPECIAL(si);
    QWORD_REG_SPECIAL(di);
    QWORD_REG_SPECIAL(bp);
    QWORD_REG_SPECIAL(sp);

    QWORD_REG_SPECIAL(ip);

    QWORD_REG_NUMBER(8);
    QWORD_REG_NUMBER(9);
    QWORD_REG_NUMBER(10);
    QWORD_REG_NUMBER(11);
    QWORD_REG_NUMBER(12);
    QWORD_REG_NUMBER(13);
    QWORD_REG_NUMBER(14);
    QWORD_REG_NUMBER(15);
  } regs;

  decoder_t *decoder;
  executor_t *executor;
  mmu_t *mmu;
};

cpu_t *new_cpu(machine_t *machine);
void free_cpu(cpu_t *cpu);
void free_cpu_ptr(cpu_t **cpu);
