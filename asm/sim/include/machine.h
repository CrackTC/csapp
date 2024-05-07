#ifndef MACHINE_H
#define MACHINE_H

typedef struct machine_t machine_t;

#include "cpu.h"
#include "elf.h"
#include "mem.h"
#include "parser.h"
#include <stddef.h>

struct machine_t {
  mem_t *mm;
  cpu_t *cpu;
  parser_t *parser;
};

machine_t *new_machine(size_t core_count, size_t mm_size);
void free_machine(machine_t *machine);
void free_machine_ptr(machine_t **machine);
void machine_run(machine_t *machine, elf_t *elf, size_t core_num);
void machine_run_text(machine_t *machine, size_t line_count, const char **lines, size_t core_num);

#endif // MACHINE_H
