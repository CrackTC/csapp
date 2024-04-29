#pragma once

#define DEFINE_CLEANUP_FUNC(T)                                                 \
  void free_##T##_ptr(T##_t **T) {                                             \
    free_##T(*T);                                                              \
    *T = (void *)0;                                                            \
  }
