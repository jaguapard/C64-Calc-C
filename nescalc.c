#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "BigNum.h"
#include "HugeNum.h"




int main()
{
    BigNum mul_res, n2, n3, n4;
    //bn_print_dec((const BigNum*)pi);
    //bn_print_dec((const BigNum*)euler);
    
    bn_set_zero(&n2);
    bn_set_zero(&n3);
    bn_set_zero(&n4);
    //'0x72f6a6' = 7534246
    n2.content[8] = 0xA6;
    n2.content[9] = 0xF6;
    n2.content[10] = 0x72;

    /*
    //result should be = PI * 7534246 = 23669531.88393828442503519227
    bn_mul((const BigNum*)pi, &n2, &mul_res);
    bn_print_dec(&mul_res);

    //Euler's number = 2.718281828459045090795598
    //Pi * Euler's number = 8.539734222673566278406671355
    bn_mul((const BigNum*)pi, (const BigNum*)euler, &mul_res);
    bn_print_dec(&mul_res);    
    bn_mul((const BigNum*)euler, (const BigNum*)pi, &mul_res);
    bn_print_dec(&mul_res);
    */
    //pi / e = 1.1557273497909217
    bn_div((const BigNum*)pi, (const BigNum*)euler, &mul_res);
    bn_print_dec(&mul_res);

    /*
    n3.content[8] = 6; 
    n4.content[8] = 1;
    bn_div(&n3, &n4, &mul_res);
    bn_print_dec(&mul_res);*/

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