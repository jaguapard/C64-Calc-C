#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "BigNum.h"
#include "HugeNum.h"


static char nice_str[50], s2[50], s3[50];
static BigNum mul_res, n2, n3, n4;

int main()
{
    const BigNum *bn_pi = (const BigNum*)pi;
    const BigNum *bn_e = (const BigNum*)euler;

    printf("Hello there, I'm gonna do some math. Slowly.\n");
    printf("Pi=%s\n", bn_print_dec_nice_str(bn_pi, nice_str));
    printf("E=%s\n", bn_print_dec_nice_str(bn_e, nice_str));
    
    bn_set_zero(&n2);
    bn_set_zero(&n3);
    bn_set_zero(&n4);
    //'0x72f6a6' = 7534246
    n2.content[8] = 0xA6;
    n2.content[9] = 0xF6;
    n2.content[10] = 0x72;

    //result should be = PI * 7534246 = 23669531.88393828442503519227
    bn_mul(bn_pi, &n2, &mul_res);
    printf("Pi*%s=%s\n", bn_print_dec_nice_str(&n2, nice_str), bn_print_dec_nice_str(&mul_res, s2));

    //Euler's number = 2.718281828459045090795598
    //Pi * Euler's number = 8.539734222673566278406671355
    bn_mul(bn_pi, bn_e, &mul_res);
    printf("Pi*E %s\n", bn_print_dec_nice_str(&mul_res, nice_str));
    
    bn_mul(bn_e, bn_pi, &mul_res);
    printf("E*Pi=%s\n", bn_print_dec_nice_str(&mul_res, nice_str));

    //pi / e = 1.1557273497909217
    bn_div(bn_pi, bn_e, &mul_res);
    printf("Pi/E=%s\n", bn_print_dec_nice_str(&mul_res, nice_str));

    bn_div(bn_e, bn_pi, &mul_res);
    printf("E/Pi=%s\n", bn_print_dec_nice_str(&mul_res, nice_str));

    bn_div(bn_pi, &n2, &mul_res);
    printf("Pi/%s=%s\n",  bn_print_dec_nice_str(&n2, nice_str), bn_print_dec_nice_str(&mul_res, s2));
    
    bn_div(bn_e, &n2, &mul_res);
    printf("E/%s=%s\n",  bn_print_dec_nice_str(&n2, nice_str), bn_print_dec_nice_str(&mul_res, s2));
    
    
    
    bn_set_zero(&n3);
    bn_set_zero(&n4);
    n3.content[0] = 0x34;
    n3.content[1] = 0x23;
    n3.content[2] = 0x11;

    n4.content[0] = 0x53;
    n4.content[1] = 0x16;
    n4.content[2] = 0xC3;
    n4.content[3] = 0xDD;
    n4.content[4] = 0xEF;

    bn_add(&n3, &n4, &mul_res);
    printf("%s+%s=%s\n", bn_print_dec_nice_str(&n3, nice_str), bn_print_dec_nice_str(&n4, s2), bn_print_dec_nice_str(&mul_res, s3) );

    bn_sub(&n4, &n3, &mul_res);
    printf("%s+%s=%s\n", bn_print_dec_nice_str(&n4, nice_str), bn_print_dec_nice_str(&n3, s2), bn_print_dec_nice_str(&mul_res, s3) );
    return 0;
}