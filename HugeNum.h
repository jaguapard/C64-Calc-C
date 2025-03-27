#ifndef HUGE_NUM_H
#define HUGE_NUM_H

#include "BigNum.h"

typedef struct
{
    uint8_t content[2*BIG_NUM_SIZE];
} HugeNum;  //special double size version for intermediate results. Not meant to be used by itself

static void huge_num_from_bn(const BigNum* inp, HugeNum* out) //copies BigNum into proper place
{
    int i;
    memset(out->content, 0, sizeof(out->content));
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        out->content[i+BIG_NUM_SIZE/2] = inp->content[i];
    }
}

//copies bytes from BigNum, meaning that result is much smaller than input if converted to decimal. It is not meant to be used for anything outside internal operations
static void huge_num_bn_raw(const BigNum* inp, HugeNum* out)
{
    int i;
    for (i = 0; i < BIG_NUM_SIZE; ++i) out->content[i] = inp->content[i];
    for (i = BIG_NUM_SIZE; i < HUGE_NUM_SIZE; ++i) out->content[i] = 0;
}

#endif