#pragma once

#include <string.h>

void
bytes_to_uint16(unsigned char **b, uint16_t *v)
{
    memcpy(v, *b, sizeof(uint16_t));

    *b += sizeof(uint16_t);
}

void
bytes_to_uint32(unsigned char **b, uint32_t *v)
{
    memcpy(v, *b, sizeof(uint32_t));

    *b += sizeof(uint32_t);
}

// void
// bytes_to_int32(unsigned char **b, int32_t *v)
// {
//     memcpy(v, *b, sizeof(int32_t));

//     *b += sizeof(int32_t);
// }

void
bytes_to_float(unsigned char **b, float *v)
{
    memcpy(v, *b, sizeof(float));

    *b += sizeof(float);
}

void
bytes_to_char(unsigned char **b, char *v, size_t size)
{
    memcpy(v, *b, size);

    *b += size;
}

uintptr_t
float2uintp(float val)
{
    union {
        uintptr_t i;
        float d;
    } convert;

    convert.d = val;

    return convert.i;
}

float
uintp2float(uintptr_t val)
{
    union {
        uintptr_t i;
        float d;
    } convert;

    convert.i = val;

    return convert.d;
}
