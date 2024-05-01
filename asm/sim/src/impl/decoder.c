#include "decoder.h"
#include "common.h"
#include "inst.h"
#include "mmu.h"
#include "reg.h"
#include <stdint.h>
#include <stdlib.h>

struct decoder_t {
  void *reg_ref;
  mmu_t *mmu_ref;
};

decoder_t *new_decoder(reg_t *reg, mmu_t *mmu) {
  decoder_t *result = malloc(sizeof(decoder_t));
  result->reg_ref = reg;
  result->mmu_ref = mmu;
  return result;
}

void free_decoder(decoder_t *dec) { free(dec); }

DEFINE_CLEANUP_FUNC(decoder)

void *decoder_decode_od(decoder_t *dec, od_t *operand) {
  if (operand->type == IMM) {
    return &operand->imm;
  }

  if (operand->type == REG) {
    return dec->reg_ref + operand->reg1_offset;
  }

  uint64_t offset = 0;
  if (operand->type & REG2) {
    offset += *(uint64_t *)(dec->reg_ref + operand->reg2_offset);
  }
  if (operand->type & SCAL) {
    offset *= operand->scal;
  }
  if (operand->type & REG1) {
    offset += *(uint64_t *)(dec->reg_ref + operand->reg1_offset);
  }
  if (operand->type & IMM) {
    offset += operand->imm;
  }

  return mmu_va2pa(dec->mmu_ref, offset);
}
