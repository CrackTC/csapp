#ifndef UTILS_H
#define UTILS_H

#define DEFINE_CLEANUP_FUNC(T)                                                 \
  void free_##T##_ptr(T##_t **(T)) {                                           \
    free_##T(*(T));                                                            \
    *(T) = (void *)0;                                                          \
  }

#define CLEANUP(x) __attribute__((cleanup(x)))

#define SIZE(M) sizeof(((reg_t *)0)->M)
#define OFFSET(M) offsetof(reg_t, M)

#define REG_MASK(R)                                                            \
  (((1ull << (SIZE(R) * 8 - 1)) - 1) | 1ull << (SIZE(R) * 8 - 1))

#define PERCENT(R)                                                             \
  { .type = REG, .reg1_offset = OFFSET(R), .reg1_mask = REG_MASK(R) }
#define DOLLAR(I)                                                              \
  { .type = IMM, .imm = (I) }

#define COMMA ,
#define EXPAND(MACRO, ...) MACRO(__VA_ARGS__)
#define THIRD(first, second, third, ...) third

#define OPTIONAL_ARG(...) , ##__VA_ARGS__

#define OPTIONAL_HELPER(THING, ...) THIRD(first, ##__VA_ARGS__, THING, )
#define OPTIONAL(THING, COND) EXPAND(OPTIONAL_HELPER, THING OPTIONAL_ARG(COND))

#define OPTIONAL_COMMA_HELPER(...) THIRD(first, ##__VA_ARGS__, COMMA, )
#define OPTIONAL_COMMA(COND) OPTIONAL_COMMA_HELPER(COND)

#define MM_TYPE_CONCAT(DISP, BASE, INDEX, SCALE) MM##DISP##BASE##INDEX##SCALE

#define MM_TYPE(DISP, BASE, INDEX, SCALE)                                      \
  EXPAND(MM_TYPE_CONCAT, OPTIONAL(_DISP, DISP), OPTIONAL(_BASE, BASE),         \
         OPTIONAL(_INDEX, INDEX), OPTIONAL(_SCALE, SCALE))

// clang-format off
#define EFFECTIVE_HELPER(DISP, BASE, INDEX, SCALE, ...)                        \
  {                                                                            \
    .type = MM_TYPE(DISP, BASE, INDEX, SCALE),                                 \
    OPTIONAL(.imm = DISP, DISP)                        OPTIONAL_COMMA(DISP)    \
    OPTIONAL(.reg1_offset = OFFSET(BASE), BASE)        OPTIONAL_COMMA(BASE)    \
    OPTIONAL(.reg1_mask = 0, BASE)                     OPTIONAL_COMMA(BASE)    \
    OPTIONAL(.reg2_offset = OFFSET(INDEX), INDEX)      OPTIONAL_COMMA(INDEX)   \
    OPTIONAL(.scal = SCALE, SCALE)                     OPTIONAL_COMMA(SCALE)   \
  }
// clang-format on

#define EFFECTIVE(...) EFFECTIVE_HELPER(__VA_ARGS__, , , )

#define READ_MASK(PTR, MASK) (*(uint64_t *)(PTR) & (MASK))
#define WRITE_MASK(PTR, VAL, MASK)                                             \
  *(uint64_t *)(PTR) = (*(uint64_t *)(PTR) & ~(MASK)) | ((VAL) & (MASK))

#endif // UTILS_H
