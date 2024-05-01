#include "decoder.h"
#include "inst.h"
#include "mmu.h"
#include "utils.h"
#include <stdlib.h>

struct decoder_t {
  cpu_t *cpu;
};

decoder_t *new_decoder(cpu_t *cpu) {
  decoder_t *result = malloc(sizeof(decoder_t));
  result->cpu = cpu;
  return result;
}

void free_decoder(decoder_t *dec) { free(dec); }

DEFINE_CLEANUP_FUNC(decoder)

void *decoder_decode_od(decoder_t *dec, od_t *od) {
  if (od->type == IMM)
    return &od->imm;

  if (od->type == REG)
    return (void *)&dec->cpu->regs + od->reg1_offset;

  uint64_t offset = 0;
  if (od->type & REG2)
    offset += *(uint64_t *)((void *)&dec->cpu->regs + od->reg2_offset);
  if (od->type & SCAL)
    offset *= od->scal;
  if (od->type & REG1)
    offset += *(uint64_t *)((void *)&dec->cpu->regs + od->reg1_offset);
  if (od->type & IMM)
    offset += od->imm;

  return mmu_va2pa(dec->cpu->mmu, offset);
}
