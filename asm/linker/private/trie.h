#ifndef TRIE_H
#define TRIE_H

typedef struct trie_t trie_t;
typedef struct trie_enumerator_t trie_enumerator_t;

trie_t *new_trie();
void trie_set(trie_t *trie, const char *key, void *value);
void *trie_get(trie_t *trie, const char *key);
void trie_remove(trie_t *trie, const char *key);
void free_trie(trie_t *trie);
trie_enumerator_t *trie_enumerate_start(trie_t *trie);
int trie_enumerator_has_value(trie_enumerator_t *enumerator);
char *trie_enumerator_get_key(trie_enumerator_t *enumerator);
void *trie_enumerator_get_value(trie_enumerator_t *enumerator);
void trie_enumerator_next(trie_enumerator_t *enumerator);
void free_trie_enumerator(trie_enumerator_t *enumerator);
void free_trie_ptr(trie_t **trie);
void free_trie_enumerator_ptr(trie_enumerator_t **trie_enumerator);

#endif // TRIE_H
