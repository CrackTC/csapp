#ifndef LINKER_H
#define LINKER_H

#include "elf_info.h"
#include <stddef.h>

int link_objects(elf_t **srcs, size_t n, elf_t *dst);
int link_executable(elf_t **srcs, size_t n, elf_t *dst);

#endif // LINKER_H
