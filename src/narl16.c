/* 
James Clarke 20/05/19 
assembler.c
This file is the first standard emulator implementation of the narl16 ISA.
*/

#include <stdio.h>
#include <string.h>

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

Cpu cpu;

#define GET_PC(cpu) ((cpu)->registers[0])

int load_prog(char * name)
{   
    
    // testing to see if written properly
    FILE * read_file;
    read_file=fopen(name,"rb");
    unsigned short buffer[8];
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
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;};
    return 0;
}