#include "executor.h"
#include "mmu.h"
#include "utils.h"
#include <stdio.h>
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

static void push(cpu_t *cpu, void *src, void *dst) {
  cpu->regs.rsp -= 8;
  *(uint64_t *)mmu_va2pa(cpu->mmu, cpu->regs.rsp) = *(uint64_t *)src;
}

static void pop(cpu_t *cpu, void *src, void *dst) {
  *(uint64_t *)src = *(uint64_t *)mmu_va2pa(cpu->mmu, cpu->regs.rsp);
  cpu->regs.rsp += 8;
}

static void call(cpu_t *cpu, void *src, void *dst) {
  push(cpu, &cpu->regs.rip, NULL);
  cpu->regs.rip = *(uint64_t *)src;
}

static void ret(cpu_t *cpu, void *src, void *dst) {
  pop(cpu, &cpu->regs.rip, NULL);
}

static void mov(cpu_t *cpu, void *src, void *dst) {
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void movl(cpu_t *cpu, void *src, void *dst) {
  *(uint32_t *)dst = *(uint32_t *)src;
}

static void movq(cpu_t *cpu, void *src, void *dst) {
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void add(cpu_t *cpu, void *src, void *dst) {
  *(uint64_t *)dst += *(uint64_t *)src;
}

static void sub(cpu_t *cpu, void *src, void *dst) {
  *(uint64_t *)dst -= *(uint64_t *)src;
}

static void dbg(cpu_t *cpu, void *src, void *dst) {
  static const char *CSI = "\33[";
  printf("%s%s", CSI, "32m");
  printf("> dbg: 0x%lx\n", cpu->regs.rdi);
  printf("%s%s", CSI, "0m");
}
static void xor (cpu_t * cpu, void *src,
                 void *dst) { *(uint64_t *)dst ^= *(uint64_t *)src; }

    handler_t handlers[] = {
    [PUSH] = push, [POP] = pop,   [CALL] = call, [RET] = ret,
    [MOV] = mov,   [MOVL] = movl, [MOVQ] = movq, [ADD] = add,
    [SUB] = sub,   [XOR] = xor,   [DBG] = dbg,
};

void executor_exec(executor_t *executor, op_t op, void *src, void *dst) {
  handlers[op](executor->cpu, src, dst);
}
