#ifndef CODE_H
#define CODE_H
#include "struct_typedef.h"

typedef struct {
    uint8_t *value;
    uint8_t length;
    uint8_t curser;
} data_t;

typedef union {
  uint8_t byte[4];
  fp32 val;
} CharAryfp32;

typedef union {
  uint8_t byte[8];
  fp32 val;
} CharAryfp64;

typedef union {
  uint8_t byte[8];
  uint64_t val;
} CharAryU64;

extern CharAryfp32 float_val;
extern CharAryfp64 double_val;
extern CharAryU64  u64_val;

#endif
