#include "encoder.h"

extern CharAryfp32 float_val;
extern CharAryfp64 double_val;
extern CharAryU64  u64_val;

void set_int8_t(data_t *data, int8_t *value, uint8_t count) {
    if (count > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
    }
}

void set_int16_t(data_t *data, int16_t *value, uint8_t count) {
    if (count * 2 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
        data->value[data->curser++] = value[i] >> 8;
    }
}

void set_int32_t(data_t *data, int32_t *value, uint8_t count) {
    if (count * 4 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
        data->value[data->curser++] = value[i] >> 8;
        data->value[data->curser++] = value[i] >> 16;
        data->value[data->curser++] = value[i] >> 24;
    }
}

void set_int64_t(data_t *data, int64_t *value, uint8_t count) {
    if (count * 8 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
        data->value[data->curser++] = value[i] >> 8;
        data->value[data->curser++] = value[i] >> 16;
        data->value[data->curser++] = value[i] >> 24;
        data->value[data->curser++] = value[i] >> 32;
        data->value[data->curser++] = value[i] >> 40;
        data->value[data->curser++] = value[i] >> 48;
        data->value[data->curser++] = value[i] >> 56;
    }
}

void set_uint8_t(data_t *data, uint8_t *value, uint8_t count) {
    if (count > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
    }
}

void set_uint16_t(data_t *data, uint16_t *value, uint8_t count) {
    if (count * 2 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
        data->value[data->curser++] = value[i] >> 8;
    }
}

void set_uint32_t(data_t *data, uint32_t *value, uint8_t count) {
    if (count * 4 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
        data->value[data->curser++] = value[i] >> 8;
        data->value[data->curser++] = value[i] >> 16;
        data->value[data->curser++] = value[i] >> 24;
    }
}

void set_uint64_t(data_t *data, uint64_t *value, uint8_t count) {
    if (count * 8 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
        data->value[data->curser++] = value[i] >> 8;
        data->value[data->curser++] = value[i] >> 16;
        data->value[data->curser++] = value[i] >> 24;
        data->value[data->curser++] = value[i] >> 32;
        data->value[data->curser++] = value[i] >> 40;
        data->value[data->curser++] = value[i] >> 48;
        data->value[data->curser++] = value[i] >> 56;
    }
}

void set_bool_t(data_t *data, bool_t *value, uint8_t count) {
    if (count > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        data->value[data->curser++] = value[i];
    }
}

void set_fp32(data_t *data, fp32 *value, uint8_t count) {
    if (count * 4 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        float_val.val = value[i];
        data->value[data->curser++] = float_val.byte[0];
        data->value[data->curser++] = float_val.byte[1];
        data->value[data->curser++] = float_val.byte[2];
        data->value[data->curser++] = float_val.byte[3];
    }
}

void set_fp64(data_t *data, fp64 *value, uint8_t count) {
    if (count * 8 > data->length - data->curser) {
        return;
    }
    for (uint8_t i = 0; i < count; i++) {
        double_val.val = value[i];
        data->value[data->curser++] = double_val.byte[0];
        data->value[data->curser++] = double_val.byte[1];
        data->value[data->curser++] = double_val.byte[2];
        data->value[data->curser++] = double_val.byte[3];
        data->value[data->curser++] = double_val.byte[4];
        data->value[data->curser++] = double_val.byte[5];
        data->value[data->curser++] = double_val.byte[6];
        data->value[data->curser++] = double_val.byte[7];
    }
}
