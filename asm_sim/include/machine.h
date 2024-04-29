#pragma once

typedef struct machine_t machine_t;

#include "cpu.h"
#include "elf.h"
#include "mm.h"

struct machine_t {
  cpu_t *cpu;
  mm_t *mm;
  elf_t *elf;
};

machine_t *new_machine(size_t mm_size);
void free_machine(machine_t *machine);
void free_machine_ptr(machine_t **machine);
void machine_load_elf(machine_t *machine, elf_t *elf);
void machine_run(machine_t *machine);
