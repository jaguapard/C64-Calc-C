#ifndef BIGNUM_H
#define BIGNUM_H
#include "base_ops.h"
#include "tables.h"
#include "HugeNum.h"

#define MIN_POWER_OF_TEN -19
#define MAX_POWER_OF_TEN 19
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

//prints decimal representation of n into str. str must have at least 42 bytes available!
static void bn_print_dec_str(const BigNum* n, char* str)
{
    BigNum remaining;
    int pwr, isLess, isGreater, isEqual, digit;
    int i = 0;
    const BigNum* p10_ptr = (const BigNum*)(powers_of_ten) - MIN_POWER_OF_TEN;

    bn_set(&remaining, n);   
    for (pwr = MAX_POWER_OF_TEN; pwr >= MIN_POWER_OF_TEN; --pwr)
    {
        const BigNum* power_of_ten = p10_ptr + pwr;

        if (pwr == -1) str[i++] = '.';       
        digit = 0;
        while (1) //repeatedly subtract powers of 10 and keep track of how much times does power of ten fit inside remaining number (that'll be decimal digits of the result)
        {
            bn_compare(&remaining, power_of_ten, &isLess, &isEqual, &isGreater);
            if (isLess) break;
            bn_sub(&remaining, power_of_ten, &remaining);
            ++digit;
        }
        if (digit > 9) digit = 9; //avoid garbage characters appearing due to precision errors on the lowest powers
        str[i++] = '0' + digit;        
    }
    str[i] = 0;
}
static void bn_print_dec(const BigNum* n)
{
    char buf[42];
    bn_print_dec_str(n, buf);
    printf("%s", buf);

    if (BN_PRINT_DEC_AUTO_NEWLINE) putchar('\n');
}

static char* bn_print_dec_nice_str(const BigNum* n, char* str)
{
    int i, j = 0, int_start = -1, fraction_end = -1;
    char sign = ' '; //TODO: get sign when it is supported
    char buf[42];

    bn_print_dec_str(n, buf);    
    for (i = 0; i < 20; ++i) //TODO: remove hardcoded stuff
    {
        if (buf[i] != '0')
        {
            //printf("Int start char is %d\n", buf[i]);
            int_start = i;
            break; 
        }
    }
    for (i = 39-3; i >= 21; --i) //-3 is here to prevent unwanted long tails: lowest numbers are basically noise due to precision errors
    {
        if (buf[i] != '0')
        {
            //printf("Frac end char is %d\n", buf[i]);
            fraction_end = i;
            break;
        }
    }

    //-1 in int_start and fraction_end means that respective part was not found (all chars are zeroes)
    if (int_start != -1) for (i = int_start; i < 20; ++i) str[j++] = buf[i];
    else str[j++] = '0';

    if (fraction_end != -1) for (i = 20; i <= fraction_end; ++i) str[j++] = buf[i];
    str[j] = 0;
    return str; //syntactic sugar for printf, doesn't actually create any new memory
}

void bn_mul(const BigNum* n1, const BigNum* n2, BigNum* out)
{
    uint16_t i;
    HugeNum shifting1, shifting2, tmp;

    //expand inputs to twice the size, since multiplication can return up to 2*x bits
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        shifting1.content[i] = n1->content[i];        
        shifting1.content[i+BIG_NUM_SIZE] = 0;
        shifting2.content[i] = n2->content[i];
        shifting2.content[i+BIG_NUM_SIZE] = 0;
    }
    memset(tmp.content, 0, sizeof(tmp.content));

    /*
        Use basic school-grade arithmetic for multiplication, but with binary. 
        Conviniently, since the only multiplier bits are 0 and 1, 
        multiplication can be replaced by chain of bit shifts and conditional additions. 
        This can probably also be sped up by multiplication table lookup, but that will take more memory.

        Example: 6 * 3 = 0b110 * 0b011
            110
        *   011
        =======
            110     = 110 * 1 (bit 0 of operand 2)
        +  110      = 110 * 1 (bit 1 of operand 2)
        + 000       = 110 * 0 (bit 2 of operand 2)
        =======
           cc       c = carry over to next digit
          10010  
        =======
        2+16=18
    */
    for (i = 0; i < (uint16_t)(HUGE_NUM_SIZE)*8; ++i)
    {
        uint8_t op2_shift_result = __chain_shift_right(shifting2.content, shifting2.content, 1, HUGE_NUM_SIZE); //if the least significant bit was set in shifting2
        if (op2_shift_result) __chain_number_add(tmp.content, shifting1.content, tmp.content, HUGE_NUM_SIZE);   //then add shifting1 to tmp
        __chain_shift_left(shifting1.content, shifting1.content, 1, HUGE_NUM_SIZE); //shift anyway
    }

    for (i = 0; i < BIG_NUM_SIZE; ++i) out->content[i] = tmp.content[i+BIG_NUM_SIZE/2];
}

void bn_div(const BigNum* n1, const BigNum* n2, BigNum* out)
{
    int i, k = 0, isLess, isEqual, isGreater;
    HugeNum shifting1, shifting2, tmp;

    memset(tmp.content, 0, sizeof(tmp.content));
    memset(shifting1.content, 0, sizeof(shifting1.content));
    for (i = 0; i < BIG_NUM_SIZE; ++i)
    {
        shifting1.content[i+BIG_NUM_SIZE/2] = n1->content[i]; //shifting1 = n1 << 64, so we can just do normal division and take the lower part
        shifting2.content[i] = n2->content[i];
        shifting2.content[i+BIG_NUM_SIZE] = 0;
    }
    
    //For example: 6 / 1. Since we have 64-bit shifted format, we need to calculate (6 << 64) / 1
    //doing this involves repeated subtraction. For that, we need to find lowest shift, that shifts divisor far enough away to become >= (6 << 64)
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

    //after this, k = 66 (1 << 66 == 4 * 1 << 64 less than or equal to 6 << 64, 1 << 67 is more)
    /*  
        use school-grade arithmetic for divison, but in binary.
        Since the only available bits of the quotient are 0 and 1, 
        that check can be performed by comparison, and acted upon by conditional subtraction

        Example: 20/3 == 10100 / 00011 = ?
            10100|00011
        -    1100|------	note the shift by 2 bits to the left
            =====|110		10100 - 1100 = 1000 == 20 - 12 = 8, bit 2 of the result is set
             1000|			shift divisor to the right
        -     110|			1000 - 110 = 10 == 8 - 6 = 2		bit 1 of the result is set
            =====|			shift again
             __10|			no longer can fit. The coded algorithm will shift and try again until no more bits are available
                                                                bit 0 of the result is cleared
    */
    for (k; k >= 0; --k)
    {
        __chain_compare(shifting1.content, shifting2.content, &isLess, &isEqual, &isGreater, HUGE_NUM_SIZE);
        if (!isLess)
        {
            __chain_number_sub(shifting1.content, shifting2.content, shifting1.content, HUGE_NUM_SIZE);            
            tmp.content[k/8] |= 1 << (k % 8); //set the k-th bit of tmp to 1
        }
        __chain_shift_right(shifting2.content, shifting2.content, 1, HUGE_NUM_SIZE);
    }

    for (i = 0; i < BIG_NUM_SIZE; ++i) out->content[i] = tmp.content[i];
    //doesn't really make sense in pseudo-real-numbered division
    //if (modulo_out) for (i = 0; i < BIG_NUM_SIZE; ++i) modulo_out->content[i] = shifting1.content[i];
}
#endif