#include "executor.h"
#include "common.h"
#include "inst.h"
#include "mmu.h"
#include "reg.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct executor_t {
  reg_t *reg_ref;
  flags_t *flags_ref;
  mmu_t *mmu_ref;
};

executor_t *new_executor(reg_t *reg, flags_t *flags, mmu_t *mmu) {
  executor_t *result = malloc(sizeof(executor_t));
  result->reg_ref = reg;
  result->flags_ref = flags;
  result->mmu_ref = mmu;
  return result;
}

void free_executor(executor_t *executor) { free(executor); }

DEFINE_CLEANUP_FUNC(executor)

#define SIGN_BIT(X, MASK) (unsigned)(((X) & (((MASK) >> 1U) + 1)) != 0)

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
  uint64_t a = READ_MASK(src, mask);
  uint64_t b = READ_MASK(dst, mask);
  uint64_t c = (a + b) & mask;
  WRITE_MASK(dst, c, mask);
  *executor->flags_ref = (flags_t){
      .cf = c < b,
      .of = ~(SIGN_BIT(b, mask) ^ SIGN_BIT(a, mask)) &
            (SIGN_BIT(b, mask) ^ SIGN_BIT(c, mask)),
      .sf = SIGN_BIT(c, mask),
      .zf = c == 0,
  };
}

static void handler_sub(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)executor;
  uint64_t a = READ_MASK(src, mask);
  uint64_t b = READ_MASK(dst, mask);
  uint64_t c = (b - a) & mask;
  WRITE_MASK(dst, c, mask);
  *executor->flags_ref = (flags_t){
      .cf = c > b,
      .of = (SIGN_BIT(b, mask) ^ SIGN_BIT(a, mask)) &
            (SIGN_BIT(b, mask) ^ SIGN_BIT(c, mask)),
      .sf = SIGN_BIT(c, mask),
      .zf = c == 0,
  };
}

static void handler_cmpq(executor_t *executor, void *src, void *dst,
                         uint64_t mask) {
  (void)executor, (void)mask;
  uint64_t a = *(uint64_t *)src;
  uint64_t b = *(uint64_t *)dst;
  uint64_t c = b - a;

  *executor->flags_ref = (flags_t){
      .cf = c > b,
      .of = ((b >> 63U) ^ (a >> 63U)) & ((b >> 63U) ^ (c >> 63U)),
      .sf = c >> 63U,
      .zf = c == 0,
  };
}

static void handler_jne(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)dst, (void)mask;
  if (executor->flags_ref->zf) {
    return;
  }
  executor->reg_ref->rip = *(uint64_t *)src;
}

static void handler_jmp(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)dst, (void)mask;
  executor->reg_ref->rip = *(uint64_t *)src;
}

static void handler_xor(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  uint64_t a = READ_MASK(src, mask);
  uint64_t b = READ_MASK(dst, mask);
  uint64_t c = a ^ b;
  WRITE_MASK(dst, c, mask);
  *executor->flags_ref = (flags_t){
      .cf = 0,
      .of = 0,
      .sf = SIGN_BIT(c, mask),
      .zf = c == 0,
  };
}

static void handler_dbg(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)src, (void)dst, (void)mask;
  static const char *CSI = "\33[";
  printf("%s%s", CSI, "32m");
  printf("> dbg: 0x%lx\n", executor->reg_ref->rdi);
  printf("%s%s", CSI, "0m");
}

static void handler_nop(executor_t *executor, void *src, void *dst,
                        uint64_t mask) {
  (void)executor, (void)src, (void)dst, (void)mask;
}

typedef void (*handler_t)(executor_t *executor, void *src, void *dst,
                          uint64_t mask);

const handler_t handlers[] = {
    [ADD] = handler_add,   [CALL] = handler_call, [CMPQ] = handler_cmpq,
    [JNE] = handler_jne,   [JMP] = handler_jmp,   [MOVL] = handler_movl,
    [MOVQ] = handler_movq, [MOV] = handler_mov,   [NOP] = handler_nop,
    [POP] = handler_pop,   [PUSH] = handler_push, [RET] = handler_ret,
    [SUB] = handler_sub,   [XOR] = handler_xor,   [DBG] = handler_dbg,
};

void executor_exec(executor_t *executor, op_t opr, void *src, void *dst,
                   uint64_t mask) {
  handlers[opr](executor, src, dst, mask);
}
