#ifndef EXECUTOR_H
#define EXECUTOR_H

typedef struct executor_t executor_t;

#include "inst.h"
#include "mmu.h"
#include "reg.h"
#include <stdint.h>

executor_t *new_executor(reg_t *reg, flags_t *flags, mmu_t *mmu);
void free_executor(executor_t *executor);
void free_executor_ptr(executor_t **executor);
void executor_exec(executor_t *executor, op_t opr, void *src, void *dst,
                   uint64_t mask);

#endif // EXECUTOR_H
