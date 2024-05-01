#ifndef INST_H
#define INST_H

#include <stddef.h>
#include <stdint.h>

typedef enum : unsigned {
  PUSH,
  POP,
  CALL,
  RET,
  MOV,
  MOVL,
  MOVQ,
  ADD,
  SUB,
  XOR,
  HALT,
  DBG
} op_t;

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
