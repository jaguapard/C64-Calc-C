#include <stdio.h>
#include <stdint.h>

#include "tables.h"

#define BIG_NUM_SIZE 16
static const char hexDigits[] = "0123456789ABCDEF";

int BN_PRINT_HEX_AUTO_NEWLINE = 1;
int BN_PRINT_HEX_SPACES = 1;
int BN_PRINT_DEC_AUTO_NEWLINE = 1;

typedef struct 
{
    uint8_t content[BIG_NUM_SIZE]; 
} BigNum;

//adds two big numbers together and returns the carry bit of the result (1 will mean that the result overflowed)
uint8_t bn_add(const BigNum* op1, const BigNum* op2, BigNum* ret)
{
    int i;
    uint16_t cast, result, carry = 0;
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        cast = op1->content[i];
        result = cast + op2->content[i] + carry;
        carry = result > 255 ? 1 : 0;
        ret->content[i] = result;
    }
    return carry;
}

//subtracts op2 from op1 and returns the carry bit of the result
uint8_t bn_sub(const BigNum* op1, const BigNum* op2, BigNum* ret)
{
    int i;
    uint16_t cast, result, carry = 0;
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        cast = op1->content[i];
        result = cast - op2->content[i] - carry;
        carry = result > 255 ? 1 : 0;
        ret->content[i] = result;
    }
    return carry;
}

//prints a BigNum in hexadecimal format, least significant bytes first, most significant bits in byte first, i.e. 65534 will be 0xFE 0xFF
void bn_print_hex(const BigNum* n)
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

void bn_set_zero(BigNum* n)
{
    int i = 0;
    for (i; i < BIG_NUM_SIZE; ++i) n->content[i] = 0;
}

void bn_set(BigNum* dst, const BigNum* src)
{
    int i = 0;
    for (i; i < BIG_NUM_SIZE; ++i) dst->content[i] = src->content[i];
}

void bn_compare(const BigNum* n1, const BigNum* n2, int* isLess, int* isEqual, int* isGreater)
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
uint8_t bn_shift_left(const BigNum* inp, BigNum* out, uint8_t shiftCount)
{
    uint8_t shiftedBit = 0;
    uint8_t i = 0, j;
    bn_set(out, inp);
    for (i; i < shiftCount; ++i)
    {
        uint8_t msb = 0;
        for (j = 0; j < BIG_NUM_SIZE; ++j)
        {
            uint8_t byte_in = out->content[j];
            out->content[j] = (byte_in << 1) | msb;
            msb = byte_in >> 7;
        }
        shiftedBit = msb;
    }
    return shiftedBit;
}

//shift BigNum right by shiftCount bits, and store result in out. Returns the last shifted out bit
uint8_t bn_shift_right(const BigNum* inp, BigNum* out, uint8_t shiftCount)
{
    uint8_t i = 0;
    int j;
    uint8_t shiftedBit = 0;
    bn_set(out, inp);
    for (i; i < shiftCount; ++i)
    {
        uint8_t lsb = 0;
        for (j = BIG_NUM_SIZE - 1; j >= 0; --j)
        {
            uint8_t byte_in = out->content[j];
            out->content[j] = (byte_in >> 1) | (lsb << 7);
            lsb = byte_in & 1;
        }
        shiftedBit = lsb;
    }
    return shiftedBit;
}

void bn_print_dec(const BigNum* n)
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
int main()
{
    BigNum mul_res, n2;
    bn_print_dec((const BigNum*)pi);
    bn_print_dec((const BigNum*)euler);
    
    bn_set_zero(&n2);
    //'0x72f6a6' = 7534246
    n2.content[0] = 0xA6;
    n2.content[1] = 0xF6;
    n2.content[2] = 0x72;

    //result should be = PI * 7534246 = 23669531.88393828442503519227
    bn_mul((const BigNum*)pi, &n2, &mul_res);
    bn_print_dec(&mul_res);

    //Euler's number = 2.718281828459045090795598
    //Pi * Euler's number = 8.539734222673566278406671355
    bn_mul((const BigNum*)pi, (const BigNum*)euler, &mul_res);
    bn_print_dec(&mul_res);    
    bn_mul((const BigNum*)euler, (const BigNum*)pi, &mul_res);
    bn_print_dec(&mul_res);
    /*
    BigNum n1, n2, n3, n4;
    bn_set_zero(&n1);
    bn_set_zero(&n2);
    n1.content[0] = 0x34;
    n1.content[1] = 0x23;
    n1.content[2] = 0x11;

    n2.content[0] = 0x53;
    n2.content[1] = 0x16;
    n2.content[2] = 0xC3;
    n2.content[3] = 0xDD;
    n2.content[4] = 0xEF;

    bn_print_hex(&n1);
    bn_print_hex(&n2);

    bn_add(&n1, &n2, &n3);
    bn_sub(&n1, &n2, &n4);
    bn_print_hex(&n3);
    bn_print_hex(&n4);

    bn_print_dec(&n1);
    bn_print_dec(&n2);
    */
    return 0;
}