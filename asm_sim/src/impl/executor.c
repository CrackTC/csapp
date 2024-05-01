#include "executor.h"
#include "mmu.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

struct executor_t {
  cpu_t *cpu;
};

executor_t *new_executor(cpu_t *cpu) {
  executor_t *result = malloc(sizeof(executor_t));
  result->cpu = cpu;
  return result;
}

void free_executor(executor_t *executor) { free(executor); }

DEFINE_CLEANUP_FUNC(executor)

static void _push(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  cpu->regs.rsp -= 8;
  *(uint64_t *)mmu_va2pa(cpu->mmu, cpu->regs.rsp) = *(uint64_t *)src;
}

static void _pop(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  *(uint64_t *)src = *(uint64_t *)mmu_va2pa(cpu->mmu, cpu->regs.rsp);
  cpu->regs.rsp += 8;
}

static void _call(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  _push(cpu, &cpu->regs.rip, NULL, mask);
  cpu->regs.rip = *(uint64_t *)src;
}

static void _ret(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  _pop(cpu, &cpu->regs.rip, NULL, mask);
}

static void _mov(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void _movl(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  *(uint32_t *)dst = *(uint32_t *)src;
}

static void _movq(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void _add(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  WRITE_MASK(dst, READ_MASK(dst, mask) + READ_MASK(src, mask), mask);
}

static void _sub(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  WRITE_MASK(dst, READ_MASK(dst, mask) - READ_MASK(src, mask), mask);
}

static void _xor(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  WRITE_MASK(dst, READ_MASK(dst, mask) ^ READ_MASK(src, mask), mask);
}

static void _dbg(cpu_t *cpu, void *src, void *dst, uint64_t mask) {
  static const char *CSI = "\33[";
  printf("%s%s", CSI, "32m");
  printf("> dbg: 0x%lx\n", cpu->regs.rdi);
  printf("%s%s", CSI, "0m");
}

typedef void (*handler_t)(cpu_t *cpu, void *src, void *dst, uint64_t mask);

handler_t handlers[] = {
    [PUSH] = _push, [POP] = _pop,   [CALL] = _call, [RET] = _ret,
    [MOV] = _mov,   [MOVL] = _movl, [MOVQ] = _movq, [ADD] = _add,
    [SUB] = _sub,   [XOR] = _xor,   [DBG] = _dbg,
};

void executor_exec(executor_t *executor, op_t op, void *src, void *dst,
                   uint64_t mask) {
  handlers[op](executor->cpu, src, dst, mask);
}
