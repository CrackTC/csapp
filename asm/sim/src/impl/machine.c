#include "machine.h"
#include "common.h"
#include "conf.h"
#include "cpu.h"
#include "decoder.h"
#include "elf.h"
#include "executor.h"
#include "inst.h"
#include "mem.h"
#include "mmu.h"
#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

machine_t *new_machine(size_t core_count, size_t mm_size) {
  machine_t *result = malloc(sizeof(machine_t));
  result->mm = new_mem(mm_size);
  result->cpu = new_cpu(result->mm, core_count);
  result->parser = new_parser();
  return result;
}

void machine_run_text(machine_t *machine, size_t line_count, const char **lines,
                      size_t core_id) {
  inst_t *insts = malloc(sizeof(inst_t) * line_count);
  size_t inst_count = 0;
  for (size_t i = 0; i < line_count; i++) {
    inst_t *inst = parser_parse_inst(machine->parser, lines[i]);
    if (inst != NULL) {
      insts[i] = *inst;
      inst_count++;
      free(inst);
    }
  }

  CLEANUP(free_elf_ptr) elf_t *elf = new_elf(insts, inst_count);
  machine_run(machine, elf, core_id);
  free(insts);
}

static void machine_load_elf(machine_t *machine, elf_t *elf) {
  for (size_t i = 0, va = EXEUTABLE_BASE; i < elf->inst_count;
       ++i, va += sizeof(uint64_t)) {
    void *pa = mmu_va2pa(machine->cpu->mmu, va);
    *(const inst_t **)pa = &elf->text[i];
  }
}

void machine_run(machine_t *machine, elf_t *elf, size_t core_id) {
  core_t *core = machine->cpu->cores[core_id];
  core->regs.rip = EXEUTABLE_BASE;
  core->regs.rsp = machine->mm->size;

  machine_load_elf(machine, elf);

  while (1) {
    const inst_t *inst =
        *(inst_t **)mmu_va2pa(machine->cpu->mmu, core->regs.rip);

    core->regs.rip += sizeof(uint64_t);

    puts(inst->code);

    if (inst->op == HLT) {
      break;
    }

    void *src = decoder_decode_od(core->decoder, &inst->src);
    void *dst = decoder_decode_od(core->decoder, &inst->dst);

    uint64_t mask = inst->src.reg1_mask | inst->dst.reg1_mask;

    executor_exec(core->executor, inst->op, src, dst, mask);
  }
}

void free_machine(machine_t *machine) {
  free_parser(machine->parser);
  free_cpu(machine->cpu);
  free_mem(machine->mm);
  free(machine);
}

DEFINE_CLEANUP_FUNC(machine);
