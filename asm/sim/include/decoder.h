#ifndef DECODER_H
#define DECODER_H

typedef struct decoder_t decoder_t;

#include "inst.h"
#include "mmu.h"
#include "reg.h"

decoder_t *new_decoder(reg_t *reg, mmu_t *mmu);
void free_decoder(decoder_t *decoder);
void free_decoder_ptr(decoder_t **decoder);
void *decoder_decode_od(decoder_t *decoder, od_t *operand);

#endif // DECODER_H
