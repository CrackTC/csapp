#include "executor.h"
#include "utils.h"
#include <stdlib.h>

struct executor_t {
  cpu_t *cpu;
};

typedef void (*handler_t)(cpu_t *cpu, void *src, void *dst);

executor_t *new_executor(cpu_t *cpu) {
  executor_t *result = malloc(sizeof(executor_t));
  result->cpu = cpu;
  return result;
}

void free_executor(executor_t *executor) { free(executor); }

DEFINE_CLEANUP_FUNC(executor)

static void mov(cpu_t *cpu, void *src, void *dst) {
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void push(cpu_t *cpu, void *src, void *dst) {
  cpu->regs.rsp -= 8;
  *(uint64_t *)mmu_va2pa(cpu->mmu, cpu->regs.rsp) = *(uint64_t *)src;
}

handler_t handlers[] = {[MOV] = mov, [PUSH] = push};

void executor_exec(executor_t *executor, op_t op, void *src, void *dst) {
  handlers[op](executor->cpu, src, dst);
}
