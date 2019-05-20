/* 
James Clarke 20/05/19 
assembler.c
This file is the first standard emulator implementation of the narl16 ISA.
*/

#include <stdio.h>

// Macros defining the base address values for areas of the memory
#define TEXT_ADDR 0x2000
#define STATIC_ADDR 0x6000
#define HEAP_ADDR 0x7000
#define STACK_ADDR 0xE000
#define VIDEO_ADDR 0xF000

// Cpu represents an instance of the narl16 CPU
typedef struct Cpu{
    // Contain register values
    int registers[12];
}Cpu;

#define GET_PC(cpu) ((cpu)->registers[0])

int main()
{
    Cpu cpu ={{0}};
    printf("%d\n",GET_PC(&cpu));
    return 0;
}