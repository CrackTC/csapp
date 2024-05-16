#include "linker/linker.h"
#include "common.h"
#include "linker/elf_info.h"
#include "list.h"
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const elf_t *elf;
  symbol_t *sym;
  uint64_t new_value;
} symbol_ref_t;

typedef struct {
  list_t *locals;
  symbol_ref_t *global;
} trie_entry_t;

static inline int global_priority(const symbol_t *sym) {
  if (sym->type == STT_NOTYPE) { /* UND */
    return 0;
  }

  if (sym->binding == STB_WEAK) { /* WEAK */
    return 1;
  }

  if (sym->section == SEC_COM) { /* COM */
    return 2;
  }

  return 3; /* GLOBAL DEFINED */
}

static inline int global_conflict(const symbol_t *new, const symbol_t *old) {
  return global_priority(new) == 3 && global_priority(old) == 3;
}

static inline int global_substitutable(const symbol_t *new,
                                       const symbol_t *old) {
  return global_priority(new) >= global_priority(old);
}

static inline symbol_ref_t *new_symbol_ref(elf_t *elf, symbol_t *sym) {
  symbol_ref_t *ref = malloc(sizeof(symbol_ref_t));
  *ref = (symbol_ref_t){.elf = elf, .sym = sym};
  return ref;
}

static inline trie_entry_t *new_trie_entry() {
  trie_entry_t *entry = malloc(sizeof(trie_entry_t));
  *entry = (trie_entry_t){0};
  entry->locals = new_list();
  return entry;
}

int resolve_syms(elf_t **srcs, size_t n, list_t **res) {
  CLEANUP(free_trie_ptr) trie_t *name2ent = new_trie();

  for (size_t i = 0; i < n; ++i) {
    elf_t *src = srcs[i];
    for (size_t j = 0; j < src->symbol_count; ++j) {
      symbol_t *cur_sym = &src->symbols[j];

      trie_entry_t *prev_syms = trie_get(name2ent, cur_sym->name);
      if (prev_syms == NULL) {
        prev_syms = new_trie_entry();
        trie_set(name2ent, cur_sym->name, prev_syms);
      }

      switch (cur_sym->binding) {
      case STB_LOCAL:
        /* add local symbol with confidence */
        list_add(prev_syms->locals, new_symbol_ref(src, cur_sym));
        break;
      case STB_WEAK:
      case STB_GLOBAL:
        if (prev_syms->global == NULL) { /* no global symbol yet, add it */
          prev_syms->global = new_symbol_ref(src, cur_sym);
        } else { /* if there is a global symbol, check if it is substitutable */
          if (global_conflict(cur_sym, prev_syms->global->sym)) { /* conflict */
            fprintf(stderr, "symbol %s has multiple definitions\n",
                    cur_sym->name);
            return -1;
          }

          if (global_substitutable(
                  cur_sym, prev_syms->global->sym)) { /* substitutable */
            /* check complex cases */
            uint64_t size1 = cur_sym->size;
            uint64_t size2 = prev_syms->global->sym->size;
            if (size1 != size2 && size1 != 0 && size2 != 0) {
              cur_sym->size = size1 > size2 ? size1 : size2;
              fprintf(stderr, "symbol %s has different sizes, using %lu\n",
                      cur_sym->name, cur_sym->size);
            }

            /* here we reuse the memory, simply overwrite the old value */
            *prev_syms->global = (symbol_ref_t){.elf = src, .sym = cur_sym};
          }

          /* otherwise we keep the old symbol */
        }
        break;
      default:
        fprintf(stderr, "unknown symbol binding %d\n", cur_sym->binding);
        return -1;
      }
    }
  }

  *res = new_list();

  /* iterate the trie and collect the result */
  TRIE_FOR(name2ent, enumerator) {
    trie_entry_t *entry = trie_enumerator_get_value(enumerator);

    list_add(*res, entry->global);
    LIST_FOR(entry->locals, node) { list_add(*res, list_data(node)); }

    free_list(entry->locals);
    free(entry);
  }

  return 0;
}

static inline int check_undefined(list_t *syms) {
  LIST_FOR(syms, node) {
    symbol_ref_t *ref = list_data(node);
    if (ref->sym->type == STT_NOTYPE) {
      fprintf(stderr, "symbol %s is undefined\n", ref->sym->name);
      return -1;
    }
  }
  return 0;
}

static inline void resolve_common(list_t *syms) {
  LIST_FOR(syms, node) {
    symbol_ref_t *ref = list_data(node);
    if (ref->sym->section == SEC_COM) {
      ref->sym->section = SEC_BSS;
    }
  }
}

/* we allocate a new symbol_t to avoid modifying the original one, which
 * may be used by original ELF */
static inline void dup_syms(list_t *syms) {
  LIST_FOR(syms, node) {
    symbol_ref_t *ref = list_data(node);

    symbol_t *new_sym = malloc(sizeof(symbol_t));
    *new_sym = *ref->sym;
    new_sym->name = strdup(ref->sym->name);
    ref->sym = new_sym;
  }
}

static inline const char *sym_section_name(symbol_ref_t *ref) {
  return ref->elf->sections[ref->sym->section].name;
}

static inline const char **sym_lines_start(symbol_ref_t *ref) {
  uint64_t section_start = ref->elf->sections[ref->sym->section].offset;
  return ref->elf->lines + section_start + ref->sym->value;
}

static inline void alloc_elf(list_t *syms, elf_t *dst) {
  CLEANUP(free_trie_ptr) trie_t *sec2size = new_trie();

  uint64_t section_count = 0;
  uint64_t bss_offset = 0; /* only one bss section */

  /* calculate the size of each section and modify the symbol value address */
  LIST_FOR(syms, node) {
    symbol_ref_t *ref = list_data(node);
    if (ref->sym->section < 0) { /* not statically alloced */
      if (ref->sym->section == SEC_BSS) {
        if (bss_offset == 0) {
          ++section_count;
        }

        ref->new_value = bss_offset;
        bss_offset += ref->sym->size;
      } else {
        ref->new_value = 0; /* no need to update the value */
      }
      continue;
    }

    const char *sec_name = sym_section_name(ref);
    uint64_t *poff = trie_get(sec2size, sec_name);
    if (poff == NULL) { /* new section */
      ++section_count;
      poff = malloc(sizeof(uint64_t));
      *poff = 0;
      trie_set(sec2size, sec_name, poff);
    }

    ref->new_value = *poff; /* update the symbol offset */
    *poff += ref->sym->size;
  }

  /* alloc the elf */
  size_t current = bss_offset == 0 ? 0 : 1;
  if (bss_offset != 0) { /* bss section */
    dst->sections[0] = (section_t){
        .name = strdup(".bss"),
        .address = 0,
        .offset = 0,
        .size = bss_offset,
    };
  }

  uint64_t start = 1;
  TRIE_FOR(sec2size, enumerator) {
    char *sec_name = trie_enumerator_get_key(enumerator);
    uint64_t *psize = trie_enumerator_get_value(enumerator);
    uint64_t size = *psize;
    *psize = current; /* here we reuse the memory to store the section index */
    dst->sections[current++] = (section_t){
        .name = sec_name,
        .address = 0,
        .offset = start,
        .size = size,
    };
    start += size;
  }
  dst->header.section_table_start = start;
  dst->header.line_count = start + section_count;
  dst->lines = malloc(sizeof(char *) * dst->header.line_count);

  current = 0;
  dst->symbol_count = list_size(syms);
  dst->symbols = malloc(sizeof(symbol_t) * dst->symbol_count);
  LIST_FOR(syms, node) {
    symbol_ref_t *ref = list_data(node);
    if (ref->sym->section < 0) {
      /* not statically alloced, just change the value address */
      dst->symbols[current++] = (symbol_t){
          .name = strdup(ref->sym->name),
          .binding = ref->sym->binding,
          .type = ref->sym->type,
          .section = ref->sym->section,
          .value = ref->new_value,
          .size = ref->sym->size,
      };
      continue;
    }

    /* otherwise, we copy corresponding lines from source elf and update the
     * section index */
    uint64_t *pindex = trie_get(sec2size, sym_section_name(ref));
    uint64_t section_offset = dst->sections[*pindex].offset;
    dst->symbols[current++] = (symbol_t){
        .name = strdup(ref->sym->name),
        .binding = ref->sym->binding,
        .type = ref->sym->type,
        .section = *pindex,
        .value = ref->new_value,
        .size = ref->sym->size,
    };
    memcpy(dst->lines + section_offset + ref->new_value, sym_lines_start(ref),
           ref->sym->size * sizeof(char *));
  }

  dst->section_count = section_count;
  dst->sections = malloc(sizeof(section_t) * section_count);

  TRIE_FOR(sec2size, enumerator) {
    free(trie_enumerator_get_value(enumerator));
  }
}

static inline void free_syms(list_t *syms) {
  LIST_FOR(syms, node) {
    symbol_ref_t *ref = list_data(node);
    free(ref->sym->name);
    free(ref->sym);
    free(ref);
  }

  free_list(syms);
}

int link_executable(elf_t **srcs, size_t n, elf_t *dst) {
  *dst = (elf_t){0};

  list_t *syms = NULL;
  if (resolve_syms(srcs, n, &syms) != 0) {
    return -1;
  }
  if (check_undefined(syms) != 0) {
    return -1;
  }
  dup_syms(syms);
  resolve_common(syms);
  alloc_elf(syms, dst);

  free_syms(syms);
  return 0;
}
