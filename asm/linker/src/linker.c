#include "linker/linker.h"
#include "common.h"
#include "linker/elf_info.h"
#include "stack.h"
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct rel_info_t rel_info_t;

struct rel_info_t {
  uint64_t elf_idx;
  uint64_t src_idx;
  uint64_t dst_idx;
  rel_info_t *see;
};

typedef struct {
  const elf_t *elf;
  sym_t *sym;
  uint64_t new_val;
  rel_info_t *rel_info;
} sym_ref_t;

typedef struct {
  stack_t *locals;
  sym_ref_t *global;
} trie_entry_t;

static inline int global_priority(const sym_t *sym) {
  if (sym->sec_idx == SEC_UND) { /* UND */
    return 0;
  }

  if (sym->binding == STB_WEAK) { /* WEAK */
    return 1;
  }

  if (sym->sec_idx == SEC_COM) { /* COM */
    return 2;
  }

  return 3; /* GLOBAL DEFINED */
}

static inline int global_conflict(const sym_t *new, const sym_t *old) {
  return global_priority(new) == 3 && global_priority(old) == 3;
}

static inline int global_substitutable(const sym_t *new, const sym_t *old) {
  return global_priority(new) >= global_priority(old);
}

static inline sym_ref_t *new_symbol_ref(elf_t *elf, sym_t *sym,
                                        rel_info_t *rel_info) {
  sym_ref_t *ref = malloc(sizeof(sym_ref_t));
  *ref = (sym_ref_t){
      .elf = elf,
      .sym = sym,
      .rel_info = rel_info,
  };
  return ref;
}

static inline rel_info_t *new_rel_info(uint64_t elf_idx, uint64_t src_idx) {
  rel_info_t *new_info = malloc(sizeof(rel_info_t));
  new_info->elf_idx = elf_idx, new_info->src_idx = src_idx,
  new_info->see = NULL;
  return new_info;
}

static inline trie_entry_t *new_trie_entry() {
  trie_entry_t *entry = malloc(sizeof(trie_entry_t));
  *entry = (trie_entry_t){0};
  entry->locals = new_stack();
  return entry;
}

int resolve_syms(elf_t **srcs, size_t n, stack_t **syms, stack_t **rels) {
  CLEANUP(free_trie_ptr) trie_t *name2ent = new_trie();

  *rels = new_stack();

  /* foreach elf */
  for (size_t elf_idx = 0; elf_idx < n; ++elf_idx) {
    elf_t *src = srcs[elf_idx];

    /* foreach symbol */
    for (size_t sym_idx = 0; sym_idx < src->symcnt; ++sym_idx) {
      sym_t *cur_sym = &src->syms[sym_idx];

      trie_entry_t *prev_syms = trie_get(name2ent, cur_sym->name);
      if (prev_syms == NULL) {
        prev_syms = new_trie_entry();
        trie_set(name2ent, cur_sym->name, prev_syms);
      }

      rel_info_t *rel_info = new_rel_info(elf_idx, sym_idx);
      switch (cur_sym->binding) {
      case STB_LOCAL:
        /* add local symbol with confidence */
        stack_add(prev_syms->locals, new_symbol_ref(src, cur_sym, rel_info));
        break;
      case STB_WEAK:
      case STB_GLOBAL:
        if (prev_syms->global == NULL) { /* no global symbol yet, add it */
          prev_syms->global = new_symbol_ref(src, cur_sym, rel_info);
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
            prev_syms->global->rel_info->see = rel_info;
            *prev_syms->global = (sym_ref_t){
                .elf = src,
                .sym = cur_sym,
                .rel_info = rel_info,
            };
          } else { /* otherwise we keep the old symbol */
            rel_info->see = prev_syms->global->rel_info;
          }
        }
        break;
      default:
        fprintf(stderr, "unknown symbol binding %d\n", cur_sym->binding);
        return -1;
      }

      stack_add(*rels, rel_info);
    }
  }

  *syms = new_stack();

  /* iterate the trie and collect the result */
  TRIE_FOR(name2ent, enumerator) {
    trie_entry_t *entry = trie_enumerator_get_value(enumerator);

    stack_add(*syms, entry->global);
    STACK_FOR(entry->locals, node) { stack_add(*syms, stack_data(node)); }

    free_stack(entry->locals);
    free(entry);
  }

  return 0;
}

static inline int chk_und(stack_t *syms) {
  STACK_FOR(syms, node) {
    sym_ref_t *ref = stack_data(node);
    if (ref->sym->sec_idx == SEC_UND) {
      fprintf(stderr, "symbol %s is undefined\n", ref->sym->name);
      return -1;
    }
  }
  return 0;
}

static inline void res_com(stack_t *syms) {
  STACK_FOR(syms, node) {
    sym_ref_t *ref = stack_data(node);
    if (ref->sym->sec_idx == SEC_COM) {
      ref->sym->sec_idx = SEC_BSS;
    }
  }
}

/* we allocate a new symbol_t to avoid modifying the original one, which
 * may be used by original ELF */
static inline void dup_syms(stack_t *syms) {
  STACK_FOR(syms, node) {
    sym_ref_t *ref = stack_data(node);

    sym_t *new_sym = malloc(sizeof(sym_t));
    *new_sym = *ref->sym;
    new_sym->name = strdup(ref->sym->name);
    ref->sym = new_sym;
  }
}

static inline const char *sym_sec_name(sym_ref_t *ref) {
  return ref->elf->secs[ref->sym->sec_idx].name;
}

static inline char **sym_l0(sym_ref_t *ref) {
  uint64_t sec_l0 = ref->elf->secs[ref->sym->sec_idx].off;
  return ref->elf->lines + sec_l0 + ref->sym->value;
}

static inline void alloc_elf(stack_t *syms, elf_t *dst, int exec) {
  CLEANUP(free_trie_ptr) trie_t *sec2size = new_trie();

  uint64_t seccnt = exec ? 1 : 2; /* reserved for .rel & .symtab section */
  uint64_t bss_offset = 0;        /* only one bss section */

  /* calculate the size of each section and modify the symbol value address */
  STACK_FOR(syms, node) {
    sym_ref_t *ref = stack_data(node);
    if (ref->sym->sec_idx < 0) { /* not statically alloced */
      if (ref->sym->sec_idx == SEC_BSS) {
        if (bss_offset == 0) {
          ++seccnt;
        }

        ref->new_val = bss_offset;
        bss_offset += ref->sym->size;
      } else {
        ref->new_val = ref->sym->value; /* no need to update the value */
      }
      continue;
    }

    const char *sec_name = sym_sec_name(ref);
    uint64_t *poff = trie_get(sec2size, sec_name);
    if (poff == NULL) { /* new section */
      ++seccnt;
      poff = malloc(sizeof(uint64_t));
      *poff = 0;
      trie_set(sec2size, sec_name, poff);
    }

    ref->new_val = *poff; /* update the symbol offset */
    *poff += ref->sym->size;
  }

  /* alloc the elf */
  dst->seccnt = seccnt;
  dst->secs = malloc(sizeof(section_t) * seccnt);

  size_t cur_idx = bss_offset == 0 ? 0 : 1;
  if (bss_offset != 0) { /* bss section */
    dst->secs[0] = (section_t){
        .name = strdup(".bss"),
        .addr = 0,
        .off = 0,
        .size = bss_offset,
    };
  }

  uint64_t cur = 1;
  TRIE_FOR(sec2size, enumerator) {
    char *sec_name = trie_enumerator_get_key(enumerator);
    uint64_t *psize = trie_enumerator_get_value(enumerator);
    uint64_t size = *psize;
    *psize = cur_idx; /* here we reuse the memory to store the section index */
    dst->secs[cur_idx++] = (section_t){
        .name = sec_name,
        .addr = 0,
        .off = cur,
        .size = size,
    };
    cur += size;
  }

  /* rel section */
  if (!exec) {
    dst->secs[cur_idx++] = (section_t){
        .name = strdup(".rel"),
        .addr = 0,
        .off = cur,
        .size = dst->relcnt,
    };
    cur += dst->relcnt;
  }

  /* symtab section */
  dst->symcnt = stack_size(syms);
  dst->syms = malloc(sizeof(sym_t) * dst->symcnt);
  dst->secs[cur_idx] = (section_t){
      .name = strdup(".symtab"),
      .addr = 0,
      .off = cur,
      .size = dst->symcnt,
  };
  cur += dst->symcnt;

  /* elf header info */
  dst->hdr.shtoff = cur;
  dst->hdr.lcnt = cur + seccnt;
  dst->lines = malloc(sizeof(char *) * dst->hdr.lcnt);
  memset(dst->lines, 0, sizeof(char *) * dst->hdr.lcnt);

  /* relocate the symbols */
  cur_idx = 0;
  STACK_FOR(syms, node) {
    sym_ref_t *ref = stack_data(node);
    ref->rel_info->dst_idx = cur_idx;
    if (ref->sym->sec_idx < 0) {
      /* not statically alloced, just change the value address */
      dst->syms[cur_idx++] = (sym_t){
          .name = strdup(ref->sym->name),
          .binding = ref->sym->binding,
          .type = ref->sym->type,
          .sec_idx = ref->sym->sec_idx,
          .value = ref->new_val,
          .size = ref->sym->size,
      };
      continue;
    }

    /* otherwise, we copy corresponding lines from source elf and update the
     * section index */
    uint64_t *pidx = trie_get(sec2size, sym_sec_name(ref));
    uint64_t sec_l0 = dst->secs[*pidx].off;
    dst->syms[cur_idx++] = (sym_t){
        .name = strdup(ref->sym->name),
        .binding = ref->sym->binding,
        .type = ref->sym->type,
        .sec_idx = *pidx,
        .value = ref->new_val,
        .size = ref->sym->size,
    };

    char **src_l0 = sym_l0(ref);
    char **dst_l0 = dst->lines + sec_l0 + ref->new_val;
    for (size_t l = 0; l < ref->sym->size; ++l) {
      *(dst_l0 + l) = strdup(*(src_l0 + l));
    }
  }

  TRIE_FOR(sec2size, enumerator) {
    free(trie_enumerator_get_value(enumerator));
  }
}

static inline void alloc_rel(elf_t **srcs, size_t n, stack_t *rels,
                             elf_t *dst) {
  uint64_t **old2new = malloc(sizeof(uint64_t *) * n);
  for (size_t elf_idx = 0; elf_idx < n; ++elf_idx) {
    old2new[elf_idx] = malloc(sizeof(uint64_t) * srcs[elf_idx]->symcnt);
  }

  STACK_FOR(rels, node) {
    rel_info_t *info = stack_data(node);
    rel_info_t *root = info;
    while (root->see != NULL) {
      root = root->see;
    }
    old2new[info->elf_idx][info->src_idx] = root->dst_idx;
  }

  dst->rels = malloc(sizeof(rel_t) * dst->relcnt);

  size_t cur = 0;
  for (size_t elf_idx = 0; elf_idx < n; ++elf_idx) {
    for (size_t rel_idx = 0; rel_idx < srcs[elf_idx]->relcnt; ++rel_idx) {
      rel_t rel = srcs[elf_idx]->rels[rel_idx];
      dst->rels[cur++] = (rel_t){
          .src_sym_idx = old2new[elf_idx][rel.src_sym_idx],
          .dst_sym_idx = old2new[elf_idx][rel.dst_sym_idx],
          .src_off = rel.src_off,
      };
    }
  }

  for (size_t elf_idx = 0; elf_idx < n; ++elf_idx) {
    free(old2new[elf_idx]);
  }
  free(old2new);
}

#define NEXT_ALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

static inline void set_addr(elf_t *dst) {
  uint64_t b0 = 0x400000;
  for (size_t sec_idx = 0; sec_idx < dst->seccnt; ++sec_idx) {
    if (strcmp(dst->secs[sec_idx].name, ".text") == 0) {
      dst->secs[sec_idx].addr = b0;
      b0 += NEXT_ALIGN(dst->secs[sec_idx].size * sizeof(uint64_t), 0x1000);
      break;
    }
  }

  for (size_t sec_idx = 0; sec_idx < dst->seccnt; ++sec_idx) {
    if (strcmp(dst->secs[sec_idx].name, ".data") == 0) {
      dst->secs[sec_idx].addr = b0;
      b0 += NEXT_ALIGN(dst->secs[sec_idx].size, 0x1000);
      break;
    }
  }

  for (size_t sec_idx = 0; sec_idx < dst->seccnt; ++sec_idx) {
    if (strcmp(dst->secs[sec_idx].name, ".bss") == 0) {
      dst->secs[sec_idx].addr = b0;
      break;
    }
  }
}

static inline size_t rel_size(const char *line) {
  size_t res = 0;
  for (const char *cur = line; *cur != '\0'; ++cur) {
    if (*cur == '<') {
      while (*cur != '>') {
        ++cur;
      }
    } else {
      ++res;
    }
  }

  return res + sizeof("$0x1122334455667788");
}

static inline void res_rel(elf_t *dst) {
  for (size_t rel_idx = 0; rel_idx < dst->relcnt; ++rel_idx) {
    rel_t rel = dst->rels[rel_idx];
    sym_t src_sym = dst->syms[rel.src_sym_idx];
    sym_t dst_sym = dst->syms[rel.dst_sym_idx];

    char **pl = dst->lines + dst->secs[src_sym.sec_idx].off + src_sym.value +
                rel.src_off;
    char *old_l = *pl;
    char *new_l = malloc(rel_size(old_l));
    *pl = new_l;

    for (char *cur = old_l; *cur != '\0'; ++cur) {
      if (*cur == '<') {
        while (*cur != '>') {
          ++cur;
        }

        new_l += sprintf(new_l, "$0x%lx",
                         dst->secs[dst_sym.sec_idx].addr + dst_sym.value);
      } else {
        *new_l++ = *cur;
      }
    }

    *new_l = '\0';
    free(old_l);
  }

  dst->relcnt = 0;
}

static inline void free_syms(stack_t *syms) {
  STACK_FOR(syms, node) {
    sym_ref_t *ref = stack_data(node);
    free(ref->sym->name);
    free(ref->sym);
    free(ref);
  }

  free_stack(syms);
}

static inline void free_rels(stack_t *rels) {
  STACK_FOR(rels, node) { free(stack_data(node)); }

  free_stack(rels);
}

static inline void cnt_rels(elf_t **srcs, size_t n, elf_t *dst) {
  dst->relcnt = 0;
  for (size_t src_idx = 0; src_idx < n; ++src_idx) {
    dst->relcnt += srcs[src_idx]->relcnt;
  }
}

int link_objects(elf_t **srcs, size_t n, elf_t *dst, int exec) {
  *dst = (elf_t){0};

  stack_t *syms = NULL;
  stack_t *rels = NULL;
  if (resolve_syms(srcs, n, &syms, &rels) != 0) {
    fprintf(stderr, "failed to resolve symbols\n");
    return -1;
  }

  if (exec && chk_und(syms) != 0) {
    fprintf(stderr, "undefined symbols\n");
    return -1;
  }

  dup_syms(syms);

  if (exec) {
    res_com(syms);
  }

  alloc_elf(syms, dst, exec);
  cnt_rels(srcs, n, dst);
  alloc_rel(srcs, n, rels, dst);

  if (exec) {
    set_addr(dst);
    res_rel(dst);
  }

  free_syms(syms);
  free_rels(rels);
  return 0;
}
