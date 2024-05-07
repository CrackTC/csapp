#ifndef COMMON_H
#define COMMON_H

#define DEFINE_CLEANUP_FUNC(T)                                                 \
  void free_##T##_ptr(T##_t **(T)) {                                           \
    free_##T(*(T));                                                            \
    *(T) = (void *)0;                                                          \
  }

#define CLEANUP(x) __attribute__((cleanup(x)))

#define COMMA ,
#define CONCAT(A, B) A##B
#define MAKE_ARG(_, A) , A
#define MAKE_ARG2(_, A) A,
#define EXPAND(MACRO, ...) MACRO(__VA_ARGS__)
#define THIRD(first, second, third, ...) third

#define OPTIONAL_ARG(...) , ##__VA_ARGS__

#define OPTIONAL_HELPER(THING, ...) THIRD(first, ##__VA_ARGS__, THING, )
#define OPTIONAL(THING, COND) EXPAND(OPTIONAL_HELPER, THING OPTIONAL_ARG(COND))

#define OPTIONAL_COMMA_HELPER(...) THIRD(first, ##__VA_ARGS__, COMMA, )
#define OPTIONAL_COMMA(COND) OPTIONAL_COMMA_HELPER(COND)

#endif // COMMON_H
