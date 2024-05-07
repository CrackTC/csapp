#include "machine.h"
#include "common.h"
#include "cpu.h"
#include "decoder.h"
#include "elf.h"
#include "executor.h"
#include "inst.h"
#include "mem.h"
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
                      size_t core_num) {
  inst_t *insts = malloc(sizeof(inst_t) * line_count);
  for (size_t i = 0; i < line_count; i++) {
    inst_t *inst = parser_parse_inst(machine->parser, lines[i]);
    insts[i] = *inst;
    free(inst);
  }

  CLEANUP(free_elf_ptr) elf_t *elf = new_elf(insts);
  machine_run(machine, elf, core_num);
}

void machine_run(machine_t *machine, elf_t *elf, size_t core_num) {
  core_t *core = machine->cpu->cores[core_num];
  core->regs.rip = 0;
  core->regs.rsp = machine->mm->size;

  while (1) {
    inst_t inst = elf->text[core->regs.rip];

    core->regs.rip++;

    puts(inst.code);

    if (inst.op == HLT) {
      break;
    }

    void *src = decoder_decode_od(core->decoder, &inst.src);
    void *dst = decoder_decode_od(core->decoder, &inst.dst);

    uint64_t mask = inst.src.reg1_mask | inst.dst.reg1_mask;

    executor_exec(core->executor, inst.op, src, dst, mask);
  }
}

void free_machine(machine_t *machine) {
  free_parser(machine->parser);
  free_cpu(machine->cpu);
  free_mem(machine->mm);
  free(machine);
}

DEFINE_CLEANUP_FUNC(machine);
