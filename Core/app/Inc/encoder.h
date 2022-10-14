#ifndef ENCODER_H
#define ENCODER_H
#include "coder.h"

extern void set_int8_t(data_t *data, int8_t *value, uint8_t count);
extern void set_int16_t(data_t *data, int16_t *value, uint8_t count);
extern void set_int32_t(data_t *data, int32_t *value, uint8_t count);
extern void set_int64_t(data_t *data, int64_t *value, uint8_t count);
extern void set_uint8_t(data_t *data, uint8_t *value, uint8_t count);
extern void set_uint16_t(data_t *data, uint16_t *value, uint8_t count);
extern void set_uint32_t(data_t *data, uint32_t *value, uint8_t count);
extern void set_uint64_t(data_t *data, uint64_t *value, uint8_t count);
extern void set_bool_t(data_t *data, bool_t *value, uint8_t count);
extern void set_fp32(data_t *data, fp32 *value, uint8_t count);
extern void set_fp64(data_t *data, fp64 *value, uint8_t count);

#endif
