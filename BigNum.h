#ifndef BIGNUM_H
#define BIGNUM_H
#include "base_ops.h"
#include "tables.h"
#include "HugeNum.h"

static int BN_PRINT_HEX_AUTO_NEWLINE = 1;
static int BN_PRINT_HEX_SPACES = 1;
static int BN_PRINT_DEC_AUTO_NEWLINE = 1;

typedef struct 
{
    uint8_t content[BIG_NUM_SIZE]; 
} BigNum;

//adds two big numbers together and returns the carry bit of the result (1 will mean that the result overflowed)
static uint8_t bn_add(const BigNum* op1, const BigNum* op2, BigNum* ret)
{
    return __chain_number_add(op1->content, op2->content, ret->content, BIG_NUM_SIZE);
}

//subtracts op2 from op1 and returns the carry bit of the result
static uint8_t bn_sub(const BigNum* op1, const BigNum* op2, BigNum* ret)
{
    return __chain_number_sub(op1->content, op2->content, ret->content, BIG_NUM_SIZE);
}

//prints a BigNum in hexadecimal format, least significant bytes first, most significant bits in byte first, i.e. 65534 will be 0xFE 0xFF
static void bn_print_hex(const BigNum* n)
{
    int i;
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        uint8_t lowNibble = n->content[i] & 0x0F;
        uint8_t highNibble = n->content[i] >> 4;
        putchar(hexDigits[highNibble]);
        putchar(hexDigits[lowNibble]);
        if (i < BIG_NUM_SIZE-1 && BN_PRINT_HEX_SPACES) putchar(' ');
    }
    if (BN_PRINT_HEX_AUTO_NEWLINE) putchar('\n');
}

static void bn_set_zero(BigNum* n)
{
    int i = 0;
    for (i; i < BIG_NUM_SIZE; ++i) n->content[i] = 0;
}

static void bn_set(BigNum* dst, const BigNum* src)
{
    int i = 0;
    for (i; i < BIG_NUM_SIZE; ++i) dst->content[i] = src->content[i];
}

static void bn_compare(const BigNum* n1, const BigNum* n2, int* isLess, int* isEqual, int* isGreater)
{
    __chain_compare(n1->content, n2->content, isLess, isEqual, isGreater, BIG_NUM_SIZE);
}

//shift BigNum left by shiftCount bits, and store result in out. Returns the last shifted out bit
static uint8_t bn_shift_left(const BigNum* inp, BigNum* out, uint8_t shiftCount)
{
    return __chain_shift_left(inp->content, out->content, shiftCount, BIG_NUM_SIZE);
}

//shift BigNum right by shiftCount bits, and store result in out. Returns the last shifted out bit
static uint8_t bn_shift_right(const BigNum* inp, BigNum* out, uint8_t shiftCount)
{
    return __chain_shift_right(inp->content, out->content, shiftCount, BIG_NUM_SIZE);
}

static void bn_print_dec(const BigNum* n)
{
    BigNum remaining;
    int pwr, isLess, isGreater, isEqual, digit;
    const BigNum* p10_ptr = (const BigNum*)(powers_of_ten) + 19;

    BN_PRINT_HEX_SPACES = 0;
    bn_set(&remaining, n);   
    for (pwr = 19; pwr >= -19; --pwr)
    {
        const BigNum* power_of_ten = p10_ptr + pwr;
        //printf("%s%d\n", "Power of ten is now ", pwr);
        //bn_print_hex(power_of_ten);

        if (pwr == -1) putchar('.');
       
        digit = 0;
        while (1)
        {
            //printf("Comparing\n");
            //bn_print_hex(&remaining);
            //bn_print_hex(power_of_ten);
            bn_compare(&remaining, power_of_ten, &isLess, &isEqual, &isGreater);
            //printf("Less: %d, Equal: %d, Greater: %d\n", isLess, isEqual, isGreater);
            if (isLess) break;
            //printf("%s%d\n", "Subbing power ", pwr);
            bn_sub(&remaining, power_of_ten, &remaining);
            ++digit;
        }
        if (digit > 9) digit = 9; //avoid garbage characters appearing due to precision errors on the lowest powers
        putchar('0'+digit);        
    }

    if (BN_PRINT_DEC_AUTO_NEWLINE) putchar('\n');
}

void bn_mul(const BigNum* n1, const BigNum* n2, BigNum* out)
{
    uint16_t i;
    HugeNum shifting1, shifting2, tmp;

    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        shifting1.content[i] = n1->content[i];        
        shifting1.content[i+BIG_NUM_SIZE] = 0;
        shifting2.content[i] = n2->content[i];
        shifting2.content[i+BIG_NUM_SIZE] = 0;
    }
    memset(tmp.content, 0, sizeof(tmp.content));

    for (i = 0; i < (uint16_t)(HUGE_NUM_SIZE)*8; ++i)
    {
        uint8_t op2_shift_result = __chain_shift_right(shifting2.content, shifting2.content, 1, HUGE_NUM_SIZE);
        if (op2_shift_result) __chain_number_add(tmp.content, shifting1.content, tmp.content, HUGE_NUM_SIZE);
        __chain_shift_left(shifting1.content, shifting1.content, 1, HUGE_NUM_SIZE);
    }

    for (i = 0; i < BIG_NUM_SIZE; ++i) out->content[i] = tmp.content[i+BIG_NUM_SIZE/2];
}

void bn_div(const BigNum* n1, const BigNum* n2, BigNum* out)
{
    
    uint16_t i, k;
    int j = 0;
    int isLess, isEqual, isGreater;
    HugeNum shifting1, shifting2, tmp;
    const uint8_t* p1 = (const uint8_t*)(&shifting1.content) + 8; 
        const uint8_t* p2 = (const uint8_t*)(&shifting2.content) + 8;
    memset(tmp.content, 0, sizeof(tmp.content));
    //tmp.content[0] = 1;
    memset(shifting1.content, 0, sizeof(shifting1.content));
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        shifting1.content[i+BIG_NUM_SIZE/2] = n1->content[i]; //shifting1 = n1 << 64, so we can just do normal division and take the lower part
        shifting2.content[i] = n2->content[i];
        shifting2.content[i+BIG_NUM_SIZE] = 0;
    }
    
    //For example: 6 / 1. Since we have 64-bit shifted format, we need to calculate (6 << 64) / 1
    //doing this involves repeated subtraction. For that, we need to find lowest shift, that shifts 1 far enough away to become >= (6 << 64)
    k = 0;
    while (1)
    {
        __chain_compare(shifting1.content, shifting2.content, &isLess, &isEqual, &isGreater, HUGE_NUM_SIZE);
        if (isGreater) 
        {
            ++k;
            __chain_shift_left(shifting2.content, shifting2.content, 1, HUGE_NUM_SIZE);
            continue;
        }
        if (isEqual)
        {
            //++k;
            break;
        }
        if (isLess)
        {
            --k;
            __chain_shift_right(shifting2.content, shifting2.content, 1, HUGE_NUM_SIZE);
            break;
        }
    }

    /*
    printf("k is %d\n", k);
    BN_PRINT_HEX_SPACES = 0;
    bn_print_hex((const BigNum*)p1);
    bn_print_hex((const BigNum*)p2);
    printf("Res\n");
    //after this, k = 66 (1 << 66 == 4 * 1 << 64 less than or equal to 6 << 64, 1 << 67 is more)
    */

    for (j = k; j >= 0; --j)
    {
        __chain_compare(shifting1.content, shifting2.content, &isLess, &isEqual, &isGreater, HUGE_NUM_SIZE);
        //printf("j=%d, L:%d, E:%d, G:%d\n", j,isLess, isEqual, isGreater);
        if (!isLess)
        {
            __chain_number_sub(shifting1.content, shifting2.content, shifting1.content, HUGE_NUM_SIZE);            
            tmp.content[j/8] |= 1 << (j % 8); //set the j-th bit of tmp to 1
        }
        __chain_shift_right(shifting2.content, shifting2.content, 1, HUGE_NUM_SIZE);
    }

    for (i = 0; i < BIG_NUM_SIZE; ++i) out->content[i] = tmp.content[i];
}
#endif