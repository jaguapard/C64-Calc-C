#ifndef BIGNUM_H
#define BIGNUM_H
#include "base_ops.h"
#include "tables.h"

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
    int i;
    for (i = BIG_NUM_SIZE-1; i >= 0; --i)
    {
        uint8_t x1 = n1->content[i];
        uint8_t x2 = n2->content[i];
        *isLess = x1 < x2;
        *isEqual = x1 == x2;
        *isGreater = x1 > x2;
        //printf("GGDsgsdfg");
        //printf("Comparing %d with %d: L:%d,E:%d,G:%d", x1, x2, *isLess, *isEqual, *isGreater);
        if (x1 != x2) break;
    }
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

//TODO: this returns n1 * n2 * 10^19 !!!
void bn_mul(const BigNum* n1, const BigNum* n2, BigNum* out)
{
    uint16_t i;
    BigNum shifting1, shifting2;

    bn_set_zero(out);
    bn_set(&shifting1, n1);
    bn_set(&shifting2, n2);
    for (i = 0; i < (uint16_t)(BIG_NUM_SIZE)*8; ++i)
    {
        uint8_t op2_shift_result = bn_shift_right(&shifting2, &shifting2, 1);
        if (op2_shift_result) bn_add(out, &shifting1, out);
        bn_shift_left(&shifting1, &shifting1, 1);
    }
}

#endif