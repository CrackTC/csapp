#pragma once

#include <stdint.h> // IWYU pragma: export

#define LO(B) uint8_t B;

#define LO_HI(B)                                                               \
  struct {                                                                     \
    uint8_t B##l;                                                              \
    uint8_t B##h;                                                              \
  };

#define QWORD_REG(Q, D, W, B_DEF)                                              \
  union {                                                                      \
    uint64_t Q;                                                                \
    uint32_t D;                                                                \
    uint16_t W;                                                                \
    B_DEF                                                                      \
  }

#define QWORD_REG_ALPHA(A) QWORD_REG(r##A##x, e##A##x, A##x, LO_HI(A))
#define QWORD_REG_SPECIAL(S) QWORD_REG(r##S, e##S, S, LO(S##l))
#define QWORD_REG_NUMBER(N) QWORD_REG(r##N, r##N##d, r##N##w, LO(r##N##b))

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
