#include "elf.h"
#include "inst.h"
#include "reg.h"
#include "utils.h"

// clang-format off
static const inst_t insts[] = {
    // main
    [0] =
    {PUSH, PERCENT(rbp), .code = "push %rbp"},
    {MOV , PERCENT(rsp), PERCENT(rbp), .code = "mov %rsp, %rbp"},
    {SUB , DOLLAR(0x20), PERCENT(rsp), .code = "sub $0x20, %rsp"},
    {MOVL, DOLLAR(0x0), EFFECTIVE(-0x4, rbp), .code = "movl $0x0, -0x4(%rbp)"},
    {MOVQ, DOLLAR(0x12340000), EFFECTIVE(-0x10, rbp), .code = "movq $0x12340000, -0x10(%rbp)"},
    {MOVQ, DOLLAR(0xabcd), EFFECTIVE(-0x18, rbp), .code = "movq $0xabcd, -0x18(%rbp)"},
    {MOV , EFFECTIVE(-0x10, rbp), PERCENT(rdi), .code = "mov -0x10(%rbp), %rdi"},
    {MOV , EFFECTIVE(-0x18, rbp), PERCENT(rsi), .code = "mov -0x18(%rbp), %rsi"},
    {CALL, DOLLAR(0x4000d0), .code = "call <add>"},
    {MOV , PERCENT(rax), PERCENT(rdi), .code = "mov %rax, %rdi"},
    {DBG , .code = "dbg"},
    {MOV , PERCENT(rax), EFFECTIVE(-0x20, rbp), .code = "mov %rax, -0x20(%rbp)"},
    {XOR , PERCENT(eax), PERCENT(eax), .code = "xor %eax, %eax"},
    {ADD , DOLLAR(0x20), PERCENT(rsp), .code = "add $0x20, %rsp"},
    {POP , PERCENT(rbp), .code = "pop %rbp"},
    {HLT, .code = "hlt"},

    // add
    [26] =
    {PUSH, PERCENT(rbp), .code = "push %rbp"},
    {MOV , PERCENT(rsp), PERCENT(rbp), .code = "mov %rsp, %rbp"},
    {MOV , PERCENT(rdi), EFFECTIVE(-0x8, rbp), .code = "mov %rdi, -0x8(%rbp)"},
    {MOV , PERCENT(rsi), EFFECTIVE(-0x10, rbp), .code = "mov %rsi, -0x10(%rbp)"},
    {MOV , EFFECTIVE(-0x8, rbp), PERCENT(rax), .code = "mov -0x8(%rbp), %rax"},
    {ADD , EFFECTIVE(-0x10, rbp), PERCENT(rax), .code = "add -0x10(%rbp), %rax"},
    {MOV , PERCENT(rax), EFFECTIVE(-0x18, rbp), .code = "mov %rax, -0x18(%rbp)"},
    {MOV , EFFECTIVE(-0x18, rbp), PERCENT(rax), .code = "mov -0x18(%rbp), %rax"},
    {POP , PERCENT(rbp), .code = "pop %rbp"},
    {RET , .code = "ret"},
};
// clang-format on

elf_t *add() { return new_elf(insts, sizeof(insts) / sizeof(inst_t)); }
