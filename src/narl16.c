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

// Macro to get the PC value
#define GET_PC() (cpu.registers[PC])
// Macro to increment the PC
#define INCREMENT_PC(ammount) ((cpu.registers[PC])+=(ammount))
// Program text
unsigned short bytecode[MAX_PROG_LEN];
// Memory is byte addressable
unsigned char * memory;

// Cpu represents an instance of the narl16 CPU
typedef struct Cpu{
    // Contain register values
    unsigned short registers[MAX_REG];
}Cpu;

Cpu cpu;

void print_registers()
{
    int i;
    for(i=0;i<MAX_REG;i++)
    {
        printf("%s: %d [%04x]\n",reg_str[i], cpu.registers[i], cpu.registers[i]);
    }
}

int execute_prog()
{
    // Run the FDE cycle
    do
    {
        unsigned short bytecode=0;
        unsigned char lower = memory[GET_PC()];
        INCREMENT_PC(1);
        unsigned char upper = memory[GET_PC()];
        INCREMENT_PC(1);
        bytecode=(bytecode|upper)<<8;
        bytecode|=lower;
        printf("bytecode: %x\n",bytecode);

        // Get the opcode and the x and y values
        unsigned char op=bytecode&0x3F,x=(bytecode>>6)&0x1F,y=(bytecode>>12)&0x1F;
        unsigned char ximmediate=0,yimmediate=0;
        // Check if we need an immediate value for x
        if(x>=20 && x<=25)
        {}
        // Check if we need an immediate value for y
        if(y>=20 && y<=25)
        {}

        switch(op)
        {
            case 0x0: break;
            case 0x1: // SET
            {break;}
            case 0x2: // ADD
            {break;}
            case 0x3: // SUB
            {break;}
            case 0x4: // MUL
            {break;}
            case 0x5: // DIV
            {break;}
            case 0x6: // AND
            {break;}
            case 0x7: // OR
            {break;}
            case 0x8: // XOR
            {break;}
            case 0x9: // NOT
            {break;}
            case 0xA: // MOD
            {break;}
            case 0xB: // REM
            {break;}
            case 0xC: // SRL
            {break;}
            case 0xD: // SLL
            {break;}
            case 0xE: // SRA
            {break;}
            case 0xF: // SLA
            {break;}
            case 0x10: // IEQ
            {break;}
            case 0x11: // INE
            {break;}
            case 0x12: // IGE
            {break;}
            case 0x13: // IGT
            {break;}
            case 0x14: // ILT
            {break;}
            case 0x15: // ILE
            {break;}
            case 0x16: // IBS
            {break;}
            case 0x17: // INB
            {break;}
            case 0x18: // JAL
            {break;}
            case 0x19: // RTN
            {break;}
            case 0x1A: // SYS
            {break;}
            case 0x1B: // INT
            {break;}
        }
        

        // Check if we have reached a NOP
        if(lower==0x0 && upper==0x0) break;
    }while(GET_PC()<MEM_SIZE);
    // Free relevant memory
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
        memory_counter+=1;
        printf("Written to memory location %x: %x\n",memory_counter,lower);
        // Get the upper 8 bits of the bytecode and write it to the next memory address
        unsigned char upper = ((bytecode[i])>>8);
        memory[memory_counter]=upper;
        memory_counter+=1;
        printf("Written to memory location %x: %x\n",memory_counter,upper);
        i++;
    }

    // Initialise CPU registers
    cpu=(Cpu){ .registers={0} };
    cpu.registers[PC]=TEXT_ADDR;
    cpu.registers[SP]=STACK_ADDR;
    print_registers();
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