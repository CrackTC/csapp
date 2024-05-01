#ifndef MEM_H
#define MEM_H

typedef struct mem_t mem_t;

#include <stddef.h>
#include <stdint.h>

struct mem_t {
  uint8_t *space;
  size_t size;
};

mem_t *new_mem(size_t size);
void free_mem(mem_t *mem);
void free_mem_ptr(mem_t **mem);

#endif // MEM_H
