#pragma once

typedef struct decoder_t decoder_t;

#include "cpu.h"
#include "inst.h"

decoder_t *new_decoder(cpu_t *cpu);
void free_decoder(decoder_t *decoder);
void free_decoder_ptr(decoder_t **decoder);
void *decoder_decode_od(decoder_t *decoder, od_t *od);
