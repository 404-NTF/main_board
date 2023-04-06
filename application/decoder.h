#ifndef DECODER_H
#define DECODER_H
#include "coder.h"

extern void get_int8_t(data_t *data, int8_t *value, uint8_t count);
extern void get_int16_t(data_t *data, int16_t *value, uint8_t count);
extern void get_int32_t(data_t *data, int32_t *value, uint8_t count);
extern void get_int64_t(data_t *data, int64_t *value, uint8_t count);
extern void get_uint8_t(data_t *data, uint8_t *value, uint8_t count);
extern void get_uint16_t(data_t *data, uint16_t *value, uint8_t count);
extern void get_uint32_t(data_t *data, uint32_t *value, uint8_t count);
extern void get_uint64_t(data_t *data, uint64_t *value, uint8_t count);
extern void get_bool_t(data_t *data, bool_t *value, uint8_t count);
extern void get_fp32(data_t *data, fp32 *value, uint8_t count);
extern void get_fp64(data_t *data, fp64 *value, uint8_t count);

#endif
