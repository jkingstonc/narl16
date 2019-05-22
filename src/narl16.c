/* 
James Clarke 20/05/19 
assembler.c
This file is the first standard emulator implementation of the narl16 ISA.
*/

#include <stdio.h>
#include <string.h>

#include "narlcodes.h"

// Macro to convert a text address position to a line number
#define ADDR_TO_LINE(addr) (((addr)-(TEXT_ADDR))/2)

// Cpu represents an instance of the narl16 CPU
typedef struct Cpu{
    // Contain register values
    int registers[12];
}Cpu;

Cpu cpu;

#define GET_PC(cpu) ((cpu)->registers[0])

int execute_prog()
{
    return 0;
}

int setup_emulator()
{
    return 0;
}

int load_prog(char * name)
{   
    // testing to see if written properly
    FILE * read_file;
    read_file=fopen(name,"rb");
    unsigned short buffer[MAX_PROG_LEN];
    fread(&buffer,8,sizeof(unsigned short),read_file);
    int i=0;
    while(buffer[i]!=0x0 || i==sizeof(buffer)/sizeof(buffer[0]))
    {
        printf("%x\n",buffer[i]);
        i++;
    }
}

int main(int argc, char *argv[])
{
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;}
    if(setup_emulator()<0) {printf("Failed to setup emulator!\n");return -1;}
    if(execute_prog()<0) {printf("Failed to execute program!\n");return -1;}
    return 0;
}