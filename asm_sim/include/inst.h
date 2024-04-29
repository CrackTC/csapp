#pragma once

#include <stdint.h>

typedef enum { MOV, PUSH, CALL, HALT } op_t;

typedef enum {
  IMM = 1,
  REG1 = 1 << 1,
  REG2 = 1 << 2,
  SCAL = 1 << 3,
  MM = 1 << 4,

  REG = REG1,
  MM_IMM = MM | IMM,
  MM_REG = MM | REG,
  MM_IMM_REG = MM | IMM | REG,
  MM_REG1_REG2 = MM | REG1 | REG2,
  MM_IMM_REG1_REG2 = MM | IMM | REG1 | REG2,
  MM_REG2_SCAL = MM | REG2 | SCAL,
  MM_IMM_REG2_SCAL = MM | IMM | REG2 | SCAL,
  MM_REG1_REG2_SCAL = MM | REG1 | REG2 | SCAL,
  MM_IMM_REG1_REG2_SCAL = MM | IMM | REG1 | REG2 | SCAL
} od_type_t;

typedef struct {
  int64_t imm;

  uint64_t *reg1;
  uint64_t *reg2;

  uint64_t scal;

  od_type_t type;
} od_t;

typedef struct {
  op_t op;
  od_t src;
  od_t dst;

  const char *code;
} inst_t;
