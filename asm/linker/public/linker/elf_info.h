#ifndef ELF_INFO_H
#define ELF_INFO_H

#include <stdint.h>
#include <stdlib.h>

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
  char *name; /* section_t owns the string */
  uint64_t addr;
  uint64_t off;
  uint64_t size; /* here size means line count */
} section_t;

typedef struct {
  char *name; /* symbol_t owns the string */
  symbol_binding_t binding;
  symbol_type_t type;
  int16_t sec_idx; /* -1: COM, -2: UND, -3: BSS */
  uint64_t value;  /* offset(lines) from section start */
  uint64_t size;   /* line count */
} sym_t;

typedef struct {
  uint64_t src_sym_idx;
  uint64_t src_off;
  uint64_t dst_sym_idx;
} rel_t;

#define SEC_COM -1
#define SEC_UND -2
#define SEC_BSS -3

typedef struct {
  uint64_t lcnt;
  uint64_t shtoff; /* offset(lines) from start of file */
} elf_header_t;

typedef struct {
  elf_header_t hdr;
  section_t *secs;
  uint64_t seccnt;
  sym_t *syms;
  uint64_t symcnt;
  rel_t *rels;
  uint64_t relcnt;
  char **lines; /* elf_t owns the pointer AND the strings */
} elf_t;

static inline void free_elf_t(elf_t *elf) {
  for (uint64_t sec_idx = 0; sec_idx < elf->seccnt; ++sec_idx) {
    free(elf->secs[sec_idx].name);
  }
  free(elf->secs);

  for (uint64_t sym_idx = 0; sym_idx < elf->symcnt; ++sym_idx) {
    free(elf->syms[sym_idx].name);
  }
  free(elf->syms);

  free(elf->rels);

  for (uint64_t l = 0; l < elf->hdr.lcnt; ++l) {
    free(elf->lines[l]);
  }
  free(elf->lines);

  free(elf);
}

#endif // ELF_INFO_H
