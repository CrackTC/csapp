#include "machine.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

machine_t *new_machine(size_t mm_size) {
  machine_t *result = malloc(sizeof(machine_t));
  result->cpu = new_cpu(result);
  result->mm = new_mm(mm_size);
  result->elf = NULL;
  return result;
}

void machine_load_elf(machine_t *machine, elf_t *elf) { machine->elf = elf; }

void machine_run(machine_t *machine) {
  machine->cpu->regs.rip = 0;
  machine->cpu->regs.rsp = machine->mm->size;

  while (1) {
    inst_t inst = machine->elf->text[machine->cpu->regs.rip];

    machine->cpu->regs.rip++;

    puts(inst.code);

    if (inst.op == HALT)
      break;

    void *src = decoder_decode_od(machine->cpu->decoder, &inst.src);
    void *dst = decoder_decode_od(machine->cpu->decoder, &inst.dst);

    uint64_t mask = inst.src.reg1_mask | inst.dst.reg1_mask;

    executor_exec(machine->cpu->executor, inst.op, src, dst, mask);
  }
}

void free_machine(machine_t *machine) {
  free_cpu(machine->cpu);
  free_mm(machine->mm);
  free(machine);
}

DEFINE_CLEANUP_FUNC(machine);
