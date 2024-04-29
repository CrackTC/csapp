#include <stdio.h>

float uint2float(unsigned u) {
  if (u == 0)
    return 0.0;

  unsigned result = 0;

  int exp = 0;

  int uu = u;
  for (int i = 31; i >= 0; i--) {
    if (uu & 0x80000000) {
      exp = i;
      break;
    }

    uu <<= 1;
  }

  result = (exp + 127) << 23;

  unsigned lower = ((1 << exp) - 1) & u;

  if (exp <= 23) {
    result |= lower << (23 - exp);
  } else {
    unsigned tmp = (lower >> (exp - 24));
    unsigned effect = tmp >> 1;

    unsigned G = effect & 0x1;
    unsigned R = tmp & 0x1;
    unsigned S = (lower & ((1 << (exp - 24)) - 1)) != 0;

    result |= effect;
    result += ((G | S) & R);
  }
  return *(float *)&result;
}

int main() {
  for (unsigned int i = 0; i < 2147483648; i++) {
    if ((float)i != uint2float(i)) {
      printf("%d: %f -> %f", i, (float)i, uint2float(i));
      return 0;
    }
  }
}
