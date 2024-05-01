#pragma once

typedef struct executor_t executor_t;

#include "cpu.h"
#include "inst.h"

executor_t *new_executor(cpu_t *cpu);
void free_executor(executor_t *executor);
void free_executor_ptr(executor_t **executor);
void executor_exec(executor_t *executor, op_t op, void *src, void *dst, uint64_t mask);
