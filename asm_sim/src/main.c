#include "conf.h"
#include "elf.h"
#include "inst.h"
#include "machine.h"
#include <stdio.h>

#define _cleanup_(x) __attribute__((cleanup(x)))

int main() {
  _cleanup_(free_machine_ptr) machine_t *machine = new_machine(MM_SIZE);

  inst_t insts[] = {{MOV,
                     {114514, .type = IMM},
                     {.reg1 = &machine->cpu->regs.rax, .type = REG},
                     "mov $0d114514, %rax"},
                    {HALT, .code = "hlt"}};

  _cleanup_(free_elf_ptr) elf_t *elf = new_elf(insts, 2);

  machine_load_elf(machine, elf);
  machine_run(machine);

  printf("%lu", machine->cpu->regs.rax);
}
