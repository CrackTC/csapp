#ifndef INST_H
#define INST_H

#include "common.h" // IWYU pragma: keep
#include <stddef.h>
#include <stdint.h>

#define FOREACH_OP(MACRO, ...)                                                 \
  MACRO(__VA_ARGS__, ADD)                                                      \
  MACRO(__VA_ARGS__, CALL)                                                     \
  MACRO(__VA_ARGS__, CMPQ)                                                     \
  MACRO(__VA_ARGS__, HLT)                                                      \
  MACRO(__VA_ARGS__, JB)                                                       \
  MACRO(__VA_ARGS__, JBR)                                                      \
  MACRO(__VA_ARGS__, JMP)                                                      \
  MACRO(__VA_ARGS__, JMPR)                                                     \
  MACRO(__VA_ARGS__, JNE)                                                      \
  MACRO(__VA_ARGS__, JNER)                                                     \
  MACRO(__VA_ARGS__, MOV)                                                      \
  MACRO(__VA_ARGS__, MOVL)                                                     \
  MACRO(__VA_ARGS__, MOVQ)                                                     \
  MACRO(__VA_ARGS__, NOP)                                                      \
  MACRO(__VA_ARGS__, POP)                                                      \
  MACRO(__VA_ARGS__, PUSH)                                                     \
  MACRO(__VA_ARGS__, RET)                                                      \
  MACRO(__VA_ARGS__, SUB)                                                      \
  MACRO(__VA_ARGS__, XOR)                                                      \
  MACRO(__VA_ARGS__, DBG)

typedef enum : unsigned { FOREACH_OP(MAKE_ARG2) } op_t;

typedef enum : unsigned {
  IMM = 1U,
  REG1 = 1U << 1U,
  REG2 = 1U << 2U,
  SCAL = 1U << 3U,
  MM = 1U << 4U,

  REG = REG1,
  MM_IMM = MM | IMM,
  MM_BASE = MM | REG,
  MM_DISP_BASE = MM | IMM | REG,
  MM_BASE_INDEX = MM | REG1 | REG2,
  MM_DISP_BASE_INDEX = MM | IMM | REG1 | REG2,
  MM_INDEX_SCALE = MM | REG2 | SCAL,
  MM_DISP_INDEX_SCALE = MM | IMM | REG2 | SCAL,
  MM_BASE_INDEX_SCALE = MM | REG1 | REG2 | SCAL,
  MM_DISP_BASE_INDEX_SCALE = MM | IMM | REG1 | REG2 | SCAL
} od_type_t;

typedef struct {
  int64_t imm;

  size_t reg1_offset;
  uint64_t reg1_mask;
  size_t reg2_offset;

  uint64_t scal;

  od_type_t type;
} od_t;

typedef struct {
  op_t op;
  od_t src;
  od_t dst;

  const char *code;
} inst_t;

#endif // INST_H
