/* 
James Clarke 20/05/19 
assembler.c
This file takes a .narl file as an input and assembles it to a binary
*/

#include <stdio.h>
#include <stdlib.h>

// Strings corresponding to indexes in the registers
const char *reg_str[] = {"pc", "sp", "ia", "cr", "r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"};

#define MAX_REG 12

int main()
{
    printf("%d\n", GET_REG_INDEX("sp"));
}