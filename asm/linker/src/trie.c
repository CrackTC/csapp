#include "trie.h"
#include "common.h"
#include "stack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct trie_t {
  void *value;
  trie_t *children[2];
};

struct trie_enumerator_t {
  trie_t *current;
  stack_t *stack;
};

trie_t *new_trie() {
  trie_t *trie = malloc(sizeof(trie_t));
  if (trie == NULL) {
    return NULL;
  }
  trie->value = NULL;
  trie->children[0] = NULL;
  trie->children[1] = NULL;
  return trie;
}

void trie_set(trie_t *trie, const char *key, void *value) {
  while (*key != '\0') {
    for (int i = 0; i < 8; ++i) {
      int bit = (*key >> (7 - i)) & 1;
      if (trie->children[bit] == NULL) {
        trie->children[bit] = new_trie();
      }
      trie = trie->children[bit];
    }
    ++key;
  }

  trie->value = value;
}

void *trie_get(trie_t *trie, const char *key) {
  while (*key != '\0') {
    for (int i = 0; i < 8; ++i) {
      int bit = (*key >> (7 - i)) & 1;
      if (trie->children[bit] == NULL) {
        return NULL;
      }
      trie = trie->children[bit];
    }
    ++key;
  }

  return trie->value;
}

void trie_remove(trie_t *trie, const char *key) {
  CLEANUP(free_stack_ptr) stack_t *path = new_stack();

  while (*key != '\0') {
    for (int i = 0; i < 8; ++i) {
      int bit = (*key >> (7 - i)) & 1;
      if (trie->children[bit] == NULL) {
        return;
      }
      trie = trie->children[bit];
      stack_add(path, trie);
    }
    ++key;
  }

  stack_node_t *node = stack_head(path);
  while (node != NULL) {
    trie_t *parent = stack_data(node);
    if (trie == parent->children[0] && parent->children[1] != NULL) {
      free_trie_ptr(&parent->children[0]);
      return;
    } else if (trie == parent->children[1] && parent->children[0] != NULL) {
      free_trie_ptr(&parent->children[1]);
      return;
    } else {
      trie = parent;
      node = stack_next(node);
    }
  }

  // no parent, so trie is the root
  trie->value = NULL;
  free_trie_ptr(&trie->children[0]);
  free_trie_ptr(&trie->children[1]);
}

void free_trie(trie_t *trie) {
  if (trie == NULL) {
    return;
  }
  for (int i = 0; i < 2; ++i) {
    free_trie(trie->children[i]);
  }
  free(trie);
}

trie_enumerator_t *trie_enumerate_start(trie_t *trie) {
  trie_enumerator_t *enumerator = malloc(sizeof(trie_enumerator_t));
  enumerator->current = trie;
  enumerator->stack = new_stack();

  if (!trie_enumerator_has_value(enumerator)) {
    trie_enumerator_next(enumerator);
  }

  return enumerator;
}

int trie_enumerator_has_value(trie_enumerator_t *enumerator) {
  return enumerator->current != NULL && enumerator->current->value != NULL;
}

void trie_enumerator_next(trie_enumerator_t *enumerator) {
  if (enumerator->current->children[0] != NULL) {
    stack_add(enumerator->stack, enumerator->current);
    enumerator->current = enumerator->current->children[0];
    if (trie_enumerator_has_value(enumerator)) {
      return;
    }
    trie_enumerator_next(enumerator);
    return;
  } else if (enumerator->current->children[1] != NULL) {
    stack_add(enumerator->stack, enumerator->current);
    enumerator->current = enumerator->current->children[1];
    if (trie_enumerator_has_value(enumerator)) {
      return;
    }
    trie_enumerator_next(enumerator);
    return;
  } else {
    stack_node_t *node = stack_head(enumerator->stack);
    while (node != NULL) {
      trie_t *parent = stack_data(node);
      if (enumerator->current == parent->children[0] &&
          parent->children[1] != NULL) {
        enumerator->current = parent->children[1];
        if (trie_enumerator_has_value(enumerator)) {
          return;
        }
        trie_enumerator_next(enumerator);
        return;
      } else {
        enumerator->current = parent;
        stack_node_t *next = stack_next(node);
        stack_remove(enumerator->stack, node);
        node = next;
      }
    }
    enumerator->current = NULL;
    return;
  }
}

void free_trie_enumerator(trie_enumerator_t *enumerator) {
  free_stack(enumerator->stack);
  free(enumerator);
}

char *trie_enumerator_get_key(trie_enumerator_t *enumerator) {
  size_t path_len = stack_size(enumerator->stack);
  assert((path_len & 7) == 0);

  size_t size = path_len >> 3;
  char *key = malloc(size + 1);
  key[size] = '\0';

  size_t current = path_len - 1;
  stack_node_t *parent = stack_head(enumerator->stack);
  trie_t *child = enumerator->current;
  while (parent != NULL) {
    trie_t *parent_node = stack_data(parent);

    if (child == parent_node->children[1]) {
      key[current >> 3] >>= 1;
      key[current >> 3] |= 0b10000000;
    } else {
      key[current >> 3] >>= 1;
      key[current >> 3] &= 0b01111111;
    }
    --current;

    child = parent_node;
    parent = stack_next(parent);
  }

  return key;
}

void *trie_enumerator_get_value(trie_enumerator_t *enumerator) {
  return enumerator->current->value;
}

DEFINE_CLEANUP_FUNC(trie)
DEFINE_CLEANUP_FUNC(trie_enumerator)
