#include "conf.h"
#include "elf.h"
#include "inst.h"
#include "machine.h"
#include "utils.h"
#include <stdio.h>

#define _cleanup_(x) __attribute__((cleanup(x)))

int main() {
  _cleanup_(free_machine_ptr) machine_t *machine = new_machine(MM_SIZE);

  cpu_t *cpu = machine->cpu;

  inst_t insts[] = {
      // main
      {PUSH, PERCENT(cpu->regs.rbp), .code = "push %rbp"},
      {MOV, PERCENT(cpu->regs.rsp), PERCENT(cpu->regs.rbp),
       .code = "mov %rsp, %rbp"},
      {SUB, DOLLAR(0x20), PERCENT(cpu->regs.rsp), .code = "sub $0x20, %rsp"},
      {MOVL, DOLLAR(0x0), EFFECTIVE(-0x4, cpu->regs.rbp),
       .code = "movl $0x0, -0x4(%rbp)"},
      {MOVQ, DOLLAR(0x12340000), EFFECTIVE(-0x10, cpu->regs.rbp),
       .code = "movq $0x12340000, -0x10(%rbp)"},
      {MOVQ, DOLLAR(0xabcd), EFFECTIVE(-0x18, cpu->regs.rbp),
       .code = "movq $0xabcd, -0x18(%rbp)"},
      {MOV, EFFECTIVE(-0x10, cpu->regs.rbp), PERCENT(cpu->regs.rdi),
       .code = "mov -0x10(%rbp), %rdi"},
      {MOV, EFFECTIVE(-0x18, cpu->regs.rbp), PERCENT(cpu->regs.rsi),
       .code = "mov -0x18(%rbp), %rsi"},
      {CALL, DOLLAR(26), .code = "call <add>"},
      {MOV, PERCENT(cpu->regs.rax), PERCENT(cpu->regs.rdi),
       .code = "mov %rax, %rdi"},
      {DBG, .code = "dbg"},
      {MOV, PERCENT(cpu->regs.rax), EFFECTIVE(-0x20, cpu->regs.rbp),
       .code = "mov %rax, -0x20(%rbp)"},
      {XOR, PERCENT(cpu->regs.eax), PERCENT(cpu->regs.eax),
       .code = "xor %eax, %eax"},
      {ADD, DOLLAR(0x20), PERCENT(cpu->regs.rsp), .code = "add $0x20, %rsp"},
      {POP, PERCENT(cpu->regs.rbp), .code = "pop %rbp"},
      {HALT, .code = "hlt"},

      // add
      [26] = {PUSH, PERCENT(cpu->regs.rbp), .code = "push %rbp"},
      {MOV, PERCENT(cpu->regs.rsp), PERCENT(cpu->regs.rbp),
       .code = "mov %rsp, %rbp"},
      {MOV, PERCENT(cpu->regs.rdi), EFFECTIVE(-0x8, cpu->regs.rbp),
       .code = "mov %rdi, -0x8(%rbp)"},
      {MOV, PERCENT(cpu->regs.rsi), EFFECTIVE(-0x10, cpu->regs.rbp),
       .code = "mov %rsi, -0x10(%rbp)"},
      {MOV, EFFECTIVE(-0x8, cpu->regs.rbp), PERCENT(cpu->regs.rax),
       .code = "mov -0x8(%rbp), %rax"},
      {ADD, EFFECTIVE(-0x10, cpu->regs.rbp), PERCENT(cpu->regs.rax),
       .code = "add -0x10(%rbp), %rax"},
      {MOV, PERCENT(cpu->regs.rax), EFFECTIVE(-0x18, cpu->regs.rbp),
       .code = "mov %rax, -0x18(%rbp)"},
      {MOV, EFFECTIVE(-0x18, cpu->regs.rbp), PERCENT(cpu->regs.rax),
       .code = "mov -0x18(%rbp), %rax"},
      {POP, PERCENT(cpu->regs.rbp), .code = "pop %rbp"},
      {RET, .code = "ret"},
  };

  _cleanup_(free_elf_ptr) elf_t *elf = new_elf(insts, 2);

  machine_load_elf(machine, elf);
  machine_run(machine);

  return 0;
}
