#pragma once

#define DEFINE_CLEANUP_FUNC(T)                                                 \
  void free_##T##_ptr(T##_t **T) {                                             \
    free_##T(*T);                                                              \
    *T = (void *)0;                                                            \
  }

#define _REG_MASK(R)                                                           \
  (((1ull << (sizeof(R) * 8 - 1)) - 1) | 1ull << (sizeof(R) * 8 - 1))

#define PERCENT(R)                                                             \
  { .type = REG, .reg1 = &R, .reg1_mask = _REG_MASK(R) }
#define DOLLAR(I)                                                              \
  { .type = IMM, .imm = I }

#define _COMMA ,
#define _EXPAND(MACRO, ...) MACRO(__VA_ARGS__)
#define _THIRD(first, second, third, ...) third

#define _OPTIONAL_ARG(...) , ##__VA_ARGS__

#define _OPTIONAL_HELPER(THING, ...) _THIRD(first, ##__VA_ARGS__, THING, )
#define _OPTIONAL(THING, COND)                                                 \
  _EXPAND(_OPTIONAL_HELPER, THING _OPTIONAL_ARG(COND))

#define _OPTIONAL_COMMA_HELPER(...) _THIRD(first, ##__VA_ARGS__, _COMMA, )
#define _OPTIONAL_COMMA(COND) _OPTIONAL_COMMA_HELPER(COND)

#define _MM_TYPE_CONCAT(DISP, BASE, INDEX, SCALE) MM##DISP##BASE##INDEX##SCALE

#define _MM_TYPE(DISP, BASE, INDEX, SCALE)                                     \
  _EXPAND(_MM_TYPE_CONCAT, _OPTIONAL(_DISP, DISP), _OPTIONAL(_BASE, BASE),     \
          _OPTIONAL(_INDEX, INDEX), _OPTIONAL(_SCALE, SCALE))

// clang-format off
#define _EFFECTIVE_HELPER(DISP, BASE, INDEX, SCALE, ...)                       \
  {                                                                            \
    .type = _MM_TYPE(DISP, BASE, INDEX, SCALE),                                \
    _OPTIONAL(.imm = DISP, DISP)                        _OPTIONAL_COMMA(DISP)  \
    _OPTIONAL(.reg1 = &BASE, BASE)                      _OPTIONAL_COMMA(BASE)  \
    _OPTIONAL(.reg1_mask = 0, BASE)                     _OPTIONAL_COMMA(BASE)  \
    _OPTIONAL(.reg2 = &INDEX, INDEX)                    _OPTIONAL_COMMA(INDEX) \
    _OPTIONAL(.scal = SCALE, SCALE)                     _OPTIONAL_COMMA(SCALE) \
  }
// clang-format on

#define EFFECTIVE(...) _EFFECTIVE_HELPER(__VA_ARGS__, , , )

#define READ_MASK(PTR, MASK) (*(uint64_t *)PTR & MASK)
#define WRITE_MASK(PTR, VAL, MASK)                                             \
  *(uint64_t *)PTR = (*(uint64_t *)PTR & ~MASK) | ((VAL) & MASK)
