#include "cpu.h"
#include "decoder.h"
#include "executor.h"
#include "machine.h"
#include "mmu.h"
#include "utils.h"
#include <stdlib.h>

cpu_t *new_cpu(machine_t *machine) {
  cpu_t *result = malloc(sizeof(cpu_t));
  result->mmu = new_mmu(machine);
  result->decoder = new_decoder(result);
  result->executor = new_executor(result);
  return result;
}

void free_cpu(cpu_t *cpu) {
  free_executor(cpu->executor);
  free_decoder(cpu->decoder);
  free_mmu(cpu->mmu);
  free(cpu);
}

DEFINE_CLEANUP_FUNC(cpu)
