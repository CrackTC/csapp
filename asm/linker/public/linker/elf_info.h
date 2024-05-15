#ifndef ELF_INFO_H
#define ELF_INFO_H

#include <stdint.h>

typedef enum {
  STB_LOCAL = 0,
  STB_WEAK = 1,
  STB_GLOBAL = 2,
} symbol_binding_t;

typedef enum {
  STT_NOTYPE = 0,
  STT_OBJECT = 1,
  STT_FUNC = 2,
} symbol_type_t;

typedef struct {
  const char *name;
  uint64_t address;
  uint64_t offset;
  uint64_t size; /* here size means line count */
} section_header_t;

typedef struct {
  const char *name;
  symbol_binding_t binding;
  symbol_type_t type;
  int16_t section; /* -1: COM, -2: UND */
  uint64_t value;  /* offset(lines) from section start */
  uint64_t size;   /* line count */
} symbol_t;

#define SEC_COM -1
#define SEC_UND -2

typedef struct {
  uint64_t line_count;
  uint64_t section_table_start; /* offset(lines) from start of file */
} elf_header_t;

typedef struct {
  elf_header_t header;
  section_header_t *section_headers;
  symbol_t *symbols;
  uint64_t symbol_count;
} elf_info_t;

#endif // ELF_INFO_H
