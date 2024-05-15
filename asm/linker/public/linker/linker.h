#ifndef LINKER_H
#define LINKER_H

#include "elf_info.h"
#include <stddef.h>

int link_elf(elf_info_t **srcs, size_t n, elf_info_t *dst);

#endif // LINKER_H
