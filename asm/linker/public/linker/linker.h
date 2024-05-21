#ifndef LINKER_H
#define LINKER_H

#include "elf_info.h"
#include <stddef.h>
#include <stdio.h>

int link_objects(elf_t **srcs, size_t n, elf_t *dst, int executable);
void write_elf(elf_t *elf, FILE *stream);

#endif // LINKER_H
