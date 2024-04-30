#include "conf.h"
#include "elf.h"
#include "inst.h"
#include "machine.h"
#include <stdio.h>

#define _cleanup_(x) __attribute__((cleanup(x)))

int main() {
  _cleanup_(free_machine_ptr) machine_t *machine = new_machine(MM_SIZE);

  cpu_t *cpu = machine->cpu;

  inst_t insts[] = {
      // main
      {PUSH, {.type = REG, .reg1 = &cpu->regs.rbp}, .code = "push %rbp"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rsp},
       {.type = REG, .reg1 = &cpu->regs.rbp},
       .code = "mov %rsp, %rbp"},
      {SUB,
       {.type = IMM, .imm = 0x20},
       {.type = REG, .reg1 = &cpu->regs.rsp},
       .code = "sub $0x20, %rsp"},
      {MOVL,
       {.type = IMM, .imm = 0x0},
       {.type = MM_IMM_REG, .imm = -0x4, .reg1 = &cpu->regs.rbp},
       .code = "movl $0x0, -0x4(%rbp)"},
      {MOVQ,
       {.type = IMM, .imm = 0x12340000},
       {.type = MM_IMM_REG, .imm = -0x10, .reg1 = &cpu->regs.rbp},
       .code = "movq $0x12340000, -0x10(%rbp)"},
      {MOVQ,
       {.type = IMM, .imm = 0xabcd},
       {.type = MM_IMM_REG, .imm = -0x18, .reg1 = &cpu->regs.rbp},
       .code = "movq $0xabcd, -0x18(%rbp)"},
      {MOV,
       {.type = MM_IMM_REG, .imm = -0x10, .reg1 = &cpu->regs.rbp},
       {.type = REG, .reg1 = &cpu->regs.rdi},
       .code = "mov -0x10(%rbp), %rdi"},
      {MOV,
       {.type = MM_IMM_REG, .imm = -0x18, .reg1 = &cpu->regs.rbp},
       {.type = REG, .reg1 = &cpu->regs.rsi},
       .code = "mov -0x18(%rbp), %rsi"},
      {CALL, {.type = IMM, .imm = 26}, .code = "call <add>"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rax},
       {.type = REG, .reg1 = &cpu->regs.rdi},
       .code = "mov %rax, %rdi"},
      {DBG, .code = "dbg"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rax},
       {.type = MM_IMM_REG, .imm = -0x20, .reg1 = &cpu->regs.rbp},
       .code = "mov %rax, -0x20(%rbp)"},
      {XOR,
       {.type = REG, .reg1 = &cpu->regs.rax},
       {.type = REG, .reg1 = &cpu->regs.rax},
       .code = "xor %eax, %eax"},
      {ADD,
       {.type = IMM, .imm = 0x20},
       {.type = REG, .reg1 = &cpu->regs.rsp},
       .code = "add $0x20, %rsp"},
      {POP, {.type = REG, .reg1 = &cpu->regs.rbp}, .code = "pop %rbp"},
      {HALT, .code = "hlt"},

      // add
      [26] = {PUSH, {.type = REG, .reg1 = &cpu->regs.rbp}, .code = "push %rbp"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rsp},
       {.type = REG, .reg1 = &cpu->regs.rbp},
       .code = "mov %rsp, %rbp"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rdi},
       {.type = MM_IMM_REG, .imm = -0x8, .reg1 = &cpu->regs.rbp},
       .code = "mov %rdi, -0x8(%rbp)"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rsi},
       {.type = MM_IMM_REG, .imm = -0x10, .reg1 = &cpu->regs.rbp},
       .code = "mov %rsi, -0x10(%rbp)"},
      {MOV,
       {.type = MM_IMM_REG, .imm = -0x8, .reg1 = &cpu->regs.rbp},
       {.type = REG, .reg1 = &cpu->regs.rax},
       .code = "mov -0x8(%rbp), %rax"},
      {ADD,
       {.type = MM_IMM_REG, .imm = -0x10, .reg1 = &cpu->regs.rbp},
       {.type = REG, .reg1 = &cpu->regs.rax},
       .code = "add -0x10(%rbp), %rax"},
      {MOV,
       {.type = REG, .reg1 = &cpu->regs.rax},
       {.type = MM_IMM_REG, .imm = -0x18, .reg1 = &cpu->regs.rbp},
       .code = "mov %rax, -0x18(%rbp)"},
      {MOV,
       {.type = MM_IMM_REG, .imm = -0x18, .reg1 = &cpu->regs.rbp},
       {.type = REG, .reg1 = &cpu->regs.rax},
       .code = "mov -0x18(%rbp), %rax"},
      {POP, {.type = REG, .reg1 = &cpu->regs.rbp}, .code = "pop %rbp"},
      {RET, .code = "ret"},
  };

  _cleanup_(free_elf_ptr) elf_t *elf = new_elf(insts, 2);

  machine_load_elf(machine, elf);
  machine_run(machine);

  printf("%lu", machine->cpu->regs.rax);
}
