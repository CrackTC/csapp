#ifndef UTILS_H
#define UTILS_H

#include "common.h"

#define SIZE(M) sizeof(((reg_t *)0)->M)
#define OFFSET(M) offsetof(reg_t, M)

#define REG_MASK(R)                                                            \
  (((1ull << (SIZE(R) * 8 - 1)) - 1) | 1ull << (SIZE(R) * 8 - 1))

#define PERCENT(R)                                                             \
  { .type = REG, .reg1_offset = OFFSET(R), .reg1_mask = REG_MASK(R) }
#define DOLLAR(I)                                                              \
  { .type = IMM, .imm = (I) }

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
