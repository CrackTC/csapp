#include "common.h"
#include "linker/elf_info.h"
#include "linker/linker.h"
#include "list.h"
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  elf_info_t *elf;
  symbol_t *sym;
} symbol_ref_t;

typedef struct {
  list_t *locals;
  symbol_ref_t *global;
} trie_entry_t;

static int global_priority(symbol_t *sym) {
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

static int global_conflict(symbol_t *new, symbol_t *old) {
  return global_priority(new) == 3 && global_priority(old) == 3;
}

static int global_substitutable(symbol_t *new, symbol_t *old) {
  return global_priority(new) >= global_priority(old);
}

static inline symbol_ref_t *new_symbol_ref(elf_info_t *elf, symbol_t *sym) {
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

int link_elf(elf_info_t **srcs, size_t n, elf_info_t *dst) {
  *dst = (elf_info_t){0};

  CLEANUP(free_trie_ptr) trie_t *trie = new_trie();

  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < srcs[i]->symbol_count; ++j) {
      symbol_t *cur_sym = &srcs[i]->symbols[j];

      trie_entry_t *prev_syms = trie_get(trie, cur_sym->name);
      if (prev_syms == NULL) {
        prev_syms = new_trie_entry();
        trie_set(trie, cur_sym->name, prev_syms);
      }

      switch (cur_sym->binding) {
      case STB_LOCAL:
        /* add local symbol with confidence */
        list_add(prev_syms->locals, new_symbol_ref(srcs[i], cur_sym));
        break;
      case STB_WEAK:
      case STB_GLOBAL:
        if (prev_syms->global == NULL) { /* no global symbol yet, add it */
          prev_syms->global = new_symbol_ref(srcs[i], cur_sym);
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
            *prev_syms->global = (symbol_ref_t){.elf = srcs[i], .sym = cur_sym};
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

  for (CLEANUP(free_trie_enumerator_ptr)
           trie_enumerator_t *enumerator = trie_enumerate_start(trie);
       trie_enumerator_has_value(enumerator);
       trie_enumerator_next(enumerator)) {
    trie_entry_t *entry = trie_enumerator_get_value(enumerator);

    /* do something */

    free_list(entry->locals);
    free(entry->global);
    free(entry);
  }

  return 0;
}
