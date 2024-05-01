#include "executor.h"
#include "inst.h"
#include "mmu.h"
#include "reg.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct executor_t {
  reg_t *reg_ref;
  mmu_t *mmu_ref;
};

executor_t *new_executor(reg_t *reg, mmu_t *mmu) {
  executor_t *result = malloc(sizeof(executor_t));
  result->reg_ref = reg;
  result->mmu_ref = mmu;
  return result;
}

void free_executor(executor_t *executor) { free(executor); }

DEFINE_CLEANUP_FUNC(executor)

static void handler_push(executor_t *executor, void *src, void *dst,
                         uint64_t mask) {
  (void)mask, (void)dst;
  executor->reg_ref->rsp -= sizeof(uint64_t);
  *(uint64_t *)mmu_va2pa(executor->mmu_ref, executor->reg_ref->rsp) =
      *(uint64_t *)src;
}

static void handler_pop(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)mask, (void)dst;
  *(uint64_t *)src =
      *(uint64_t *)mmu_va2pa(executor->mmu_ref, executor->reg_ref->rsp);
  executor->reg_ref->rsp += sizeof(uint64_t);
}

static void handler_call(executor_t *executor, void *src, void *dst,
                         uint64_t mask) {
  (void)dst;
  handler_push(executor, &executor->reg_ref->rip, NULL, mask);
  executor->reg_ref->rip = *(uint64_t *)src;
}

static void handler_ret(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)src, (void)dst;
  handler_pop(executor, &executor->reg_ref->rip, NULL, mask);
}

static void handler_mov(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)executor, (void)mask;
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void handler_movl(executor_t *executor, void *src, void *dst,
                         uint64_t mask) {
  (void)executor, (void)mask;
  *(uint32_t *)dst = *(uint32_t *)src;
}

static void handler_movq(executor_t *executor, void *src, void *dst,
                         uint64_t mask) {
  (void)executor, (void)mask;
  *(uint64_t *)dst = *(uint64_t *)src;
}

static void handler_add(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)executor;
  WRITE_MASK(dst, READ_MASK(dst, mask) + READ_MASK(src, mask), mask);
}

static void handler_sub(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)executor;
  WRITE_MASK(dst, READ_MASK(dst, mask) - READ_MASK(src, mask), mask);
}

static void handler_xor(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)executor;
  WRITE_MASK(dst, READ_MASK(dst, mask) ^ READ_MASK(src, mask), mask);
}

static void handler_dbg(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)src, (void)dst, (void)mask;
  static const char *CSI = "\33[";
  printf("%s%s", CSI, "32m");
  printf("> dbg: 0x%lx\n", executor->reg_ref->rdi);
  printf("%s%s", CSI, "0m");
}

typedef void (*handler_t)(executor_t *executor, void *src, void *dst,
                          uint64_t mask);

const handler_t handlers[] = {
    [PUSH] = handler_push, [POP] = handler_pop, [CALL] = handler_call,
    [RET] = handler_ret,   [MOV] = handler_mov, [MOVL] = handler_movl,
    [MOVQ] = handler_movq, [ADD] = handler_add, [SUB] = handler_sub,
    [XOR] = handler_xor,   [DBG] = handler_dbg,
};

void executor_exec(executor_t *executor, op_t opr, void *src, void *dst,
                   uint64_t mask) {
  handlers[opr](executor, src, dst, mask);
}