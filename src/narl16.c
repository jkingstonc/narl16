/* 
James Clarke 20/05/19 
assembler.c
This file is the first standard emulator implementation of the narl16 ISA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "narlcodes.h"

// Macro to convert a text address position to a line number
#define ADDR_TO_LINE(addr) (((addr)-(TEXT_ADDR))/2)

// Program text
unsigned short bytecode[MAX_PROG_LEN];
// Memory is byte addressable
unsigned char * memory;

// Cpu represents an instance of the narl16 CPU
typedef struct Cpu{
    // Contain register values
    int registers[MAX_REG];
}Cpu;

Cpu cpu;

#define GET_PC(cpu) ((cpu)->registers[0])

int execute_prog()
{
    free(memory);
    return 0;
}

int setup_emulator()
{
    // First setup and load the program into memory
    memory = (unsigned char*)malloc(sizeof(unsigned char)*MEM_SIZE);
    unsigned short memory_counter = TEXT_ADDR;
    int i=0;
    while(bytecode[i]!=0x0)
    {
        // Get the lower 8 bits of the bytecode and write it to the next memory address
        unsigned char lower = (bytecode[i])&0xFF;
        memory[memory_counter]=lower;
        memory_counter+=8;
        // Get the upper 8 bits of the bytecode and write it to the next memory address
        unsigned char upper = ((bytecode[i])>>8);
        memory[memory_counter]=upper;
        memory_counter+=8;
        i++;
    }

    // Initialise CPU registers
    cpu=(Cpu){ .registers={0} };
    cpu.registers[PC]=TEXT_ADDR;
    cpu.registers[SP]=STACK_ADDR;

    // Run the FDE cycle
    while(1)
    {}

    return 0;
}

int load_prog(char * name)
{   
    // Load the file and write the bytecode to an unsigned short array
    FILE * read_file;
    read_file=fopen(name,"rb");
    fread(&bytecode,8,sizeof(unsigned short),read_file);
}

int main(int argc, char *argv[])
{
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;}
    if(setup_emulator()<0) {printf("Failed to setup emulator!\n");return -1;}
    if(execute_prog()<0) {printf("Failed to execute program!\n");return -1;}
    return 0;
}