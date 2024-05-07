#ifndef REG_H
#define REG_H

#include <stdint.h>

#define LO_DEF(B) uint8_t B;

#define LO_HI_DEF(B)                                                           \
  struct {                                                                     \
    uint8_t B##l;                                                              \
    uint8_t B##h;                                                              \
  };

#define QWORD_REG(Q, D, W, B_DEF)                                              \
  union {                                                                      \
    B_DEF                                                                      \
    uint16_t W;                                                                \
    uint32_t D;                                                                \
    uint64_t Q;                                                                \
  }

#define QWORD_REG_ALPHA(A) QWORD_REG(r##A##x, e##A##x, A##x, LO_HI_DEF(A))
#define QWORD_REG_SPECIAL(S) QWORD_REG(r##S, e##S, S, LO_DEF(S##l))
#define QWORD_REG_NUMBER(N) QWORD_REG(r##N, r##N##d, r##N##w, LO_DEF(r##N##b))

#define APPLY_REG_ALPHA(MACRO, A, ...)                                         \
  MACRO(__VA_ARGS__, r##A##x)                                                  \
  MACRO(__VA_ARGS__, e##A##x)                                                  \
  MACRO(__VA_ARGS__, A##x)                                                     \
  MACRO(__VA_ARGS__, A##l)                                                     \
  MACRO(__VA_ARGS__, A##h)

#define APPLY_REG_SPECIAL(MACRO, S, ...)                                       \
  MACRO(__VA_ARGS__, r##S)                                                     \
  MACRO(__VA_ARGS__, e##S)                                                     \
  MACRO(__VA_ARGS__, S)                                                        \
  MACRO(__VA_ARGS__, S##l)

#define APPLY_REG_NUMBER(MACRO, N, ...)                                        \
  MACRO(__VA_ARGS__, r##N)                                                     \
  MACRO(__VA_ARGS__, r##N##d)                                                  \
  MACRO(__VA_ARGS__, r##N##w)                                                  \
  MACRO(__VA_ARGS__, r##N##b)

#define FOREACH_REG_ALPHA(MACRO, ...)                                          \
  APPLY_REG_ALPHA(MACRO, a, __VA_ARGS__)                                       \
  APPLY_REG_ALPHA(MACRO, b, __VA_ARGS__)                                       \
  APPLY_REG_ALPHA(MACRO, c, __VA_ARGS__)                                       \
  APPLY_REG_ALPHA(MACRO, d, __VA_ARGS__)

#define FOREACH_REG_SPECIAL(MACRO, ...)                                        \
  APPLY_REG_SPECIAL(MACRO, si, __VA_ARGS__)                                    \
  APPLY_REG_SPECIAL(MACRO, di, __VA_ARGS__)                                    \
  APPLY_REG_SPECIAL(MACRO, bp, __VA_ARGS__)                                    \
  APPLY_REG_SPECIAL(MACRO, sp, __VA_ARGS__)                                    \
  APPLY_REG_SPECIAL(MACRO, ip, __VA_ARGS__)

#define FOREACH_REG_NUMBER(MACRO, ...)                                         \
  APPLY_REG_NUMBER(MACRO, 8, __VA_ARGS__)                                      \
  APPLY_REG_NUMBER(MACRO, 9, __VA_ARGS__)                                      \
  APPLY_REG_NUMBER(MACRO, 10, __VA_ARGS__)                                     \
  APPLY_REG_NUMBER(MACRO, 11, __VA_ARGS__)                                     \
  APPLY_REG_NUMBER(MACRO, 12, __VA_ARGS__)                                     \
  APPLY_REG_NUMBER(MACRO, 13, __VA_ARGS__)                                     \
  APPLY_REG_NUMBER(MACRO, 14, __VA_ARGS__)                                     \
  APPLY_REG_NUMBER(MACRO, 15, __VA_ARGS__)

#define FOREACH_REG(MACRO, ...)                                                \
  FOREACH_REG_ALPHA(MACRO, __VA_ARGS__)                                        \
  FOREACH_REG_SPECIAL(MACRO, __VA_ARGS__)                                      \
  FOREACH_REG_NUMBER(MACRO, __VA_ARGS__)

typedef struct {
  QWORD_REG_ALPHA(a);
  QWORD_REG_ALPHA(b);
  QWORD_REG_ALPHA(c);
  QWORD_REG_ALPHA(d);

  QWORD_REG_SPECIAL(si);
  QWORD_REG_SPECIAL(di);
  QWORD_REG_SPECIAL(bp);
  QWORD_REG_SPECIAL(sp);

  QWORD_REG_SPECIAL(ip);

  QWORD_REG_NUMBER(8);
  QWORD_REG_NUMBER(9);
  QWORD_REG_NUMBER(10);
  QWORD_REG_NUMBER(11);
  QWORD_REG_NUMBER(12);
  QWORD_REG_NUMBER(13);
  QWORD_REG_NUMBER(14);
  QWORD_REG_NUMBER(15);
} reg_t;

typedef union {
  uint8_t flags;
  struct {
    uint8_t cf : 1;
    uint8_t of : 1;
    uint8_t sf : 1;
    uint8_t zf : 1;
  };
} flags_t;

#endif // REG_H
