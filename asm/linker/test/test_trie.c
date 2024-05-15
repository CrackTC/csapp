#include "common.h"
#include "trie.h"
#include <assert.h>
#include <stdio.h>

int main() {
  CLEANUP(free_trie_ptr) trie_t *trie = new_trie();
  const char **strs =
      (const char *[]){"to", "tea", "ted", "ten", "A", "in", "inn"};
  int *vals = (int[]){7, 3, 4, 12, 15, 11, 5, 9};

  /* insertion */
  for (int i = 0; i < 7; i++) {
    trie_set(trie, strs[i], &vals[i]);
  }

  for (int i = 0; i < 7; i++) {
    int *val = trie_get(trie, strs[i]);
    assert(*val == vals[i]);
  }

  fputs("insertion test passed\n", stderr);

  /* deletion */
  trie_remove(trie, "tea");

  assert(trie_get(trie, "tea") == NULL);
  assert(trie_get(trie, "ted") != NULL);

  fputs("deletion test passed\n", stderr);

  int *remaining_vals[] = {&vals[4], &vals[5], &vals[6],
                           &vals[2], &vals[3], &vals[0]};

  /* enumeration */
  int i = 0;
  for (CLEANUP(free_trie_enumerator_ptr)
           trie_enumerator_t *enumerator = trie_enumerate_start(trie);
       trie_enumerator_has_value(enumerator);
       trie_enumerator_next(enumerator)) {
    fprintf(stderr, "enumerating %d, expected %d\n",
            *(int *)trie_enumerator_get_value(enumerator), *remaining_vals[i]);
    int *val = trie_enumerator_get_value(enumerator);
    assert(*val == *remaining_vals[i]);
    ++i;
  }

  fputs("enumeration test passed\n", stderr);
}
