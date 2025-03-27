#include <stdio.h>
#include <stdint.h>

#define BIG_NUM_SIZE 16
static const char hexDigits[] = "0123456789ABCDEF";
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
        if (i < BIG_NUM_SIZE-1) putchar(' ');
    }
}
int main()
{
    BigNum n1, n2;
    n1.content[0] = 0x34;
    n1.content[1] = 0x23;
    n1.content[2] = 0x11;

    n2.content[0] = 0x53;
    n2.content[1] = 0x16;
    n2.content[2] = 0xC3;
    n2.content[3] = 0xDD;
    n2.content[4] = 0xEF;

    bn_print_hex(&n1);
    putchar('\n');
    bn_print_hex(&n2);
    putchar('\n');
    return 0;
}