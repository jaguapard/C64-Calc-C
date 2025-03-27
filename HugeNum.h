#ifndef HUGE_NUM_H
#define HUGE_NUM_H

#include "base_ops.h"

typedef struct
{
    uint8_t content[2*BIG_NUM_SIZE];
} HugeNum;  //special double size version for intermediate results. Not meant to be used by itself

#endif