#ifndef BASE_OPS_H
#define BASE_OPS_H
#include <stdint.h>

#define BIG_NUM_SIZE 16
#define HUGE_NUM_SIZE BIG_NUM_SIZE*2

static uint8_t __chain_number_add(const uint8_t* op1, const uint8_t* op2, uint8_t* ret, int numSize)
{
    int i;
    uint16_t cast, result, carry = 0;
    for (i = 0; i < numSize; ++i)
    {
        cast = op1[i];
        result = cast + op2[i] + carry;
        carry = result > 255 ? 1 : 0;
        ret[i] = result;
    }
    return carry;
}

static uint8_t __chain_number_sub(const uint8_t* op1, const uint8_t* op2, uint8_t* ret, int numSize)
{
    int i;
    uint16_t cast, result, carry = 0;
    for (i = 0; i < numSize; ++i)
    {
        cast = op1[i];
        result = cast - op2[i] - carry;
        carry = result > 255 ? 1 : 0;
        ret[i] = result;
    }
    return carry;
}

static uint8_t __chain_shift_right(const uint8_t* inp, uint8_t* out, uint8_t shiftCount, int numSize)
{
    int i, j;
    uint8_t shiftedBit = 0;

    for (i = 0; i < numSize; ++i) out[i] = inp[i];
    for (i = 0; i < shiftCount; ++i)
    {
        uint8_t lsb = 0;
        for (j = numSize - 1; j >= 0; --j)
        {
            uint8_t byte_in = out[j];
            out[j] = (byte_in >> 1) | (lsb << 7);
            lsb = byte_in & 1;
        }
        shiftedBit = lsb;
    }
    return shiftedBit;
}

static uint8_t __chain_shift_left(const uint8_t* inp, uint8_t* out, uint8_t shiftCount, int numSize)
{
    uint8_t shiftedBit = 0;
    uint8_t i, j;
    for (i = 0; i < numSize; ++i) out[i] = inp[i];
    for (i = 0; i < shiftCount; ++i)
    {
        uint8_t msb = 0;
        for (j = 0; j < numSize; ++j)
        {
            uint8_t byte_in = out[j];
            out[j] = (byte_in << 1) | msb;
            msb = byte_in >> 7;
        }
        shiftedBit = msb;
    }
    return shiftedBit;
}

static void __chain_compare(const uint8_t* op1, const uint8_t* op2, int* isLess, int* isEqual, int* isGreater, int numSize)
{
    int i;
    for (i = numSize-1; i >= 0; --i)
    {
        uint8_t x1 = op1[i];
        uint8_t x2 = op2[i];
        *isLess = x1 < x2;
        *isEqual = x1 == x2;
        *isGreater = x1 > x2;
        //printf("GGDsgsdfg");
        //printf("Comparing %d with %d: L:%d,E:%d,G:%d", x1, x2, *isLess, *isEqual, *isGreater);
        if (x1 != x2) break;
    }
}

#endif