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
// Macro to get the SP value
#define GET_SP() (cpu.registers[SP])
// Macro to increment the PC
#define INCREMENT_PC(ammount) ((cpu.registers[PC])=(cpu.registers[PC])+(ammount))
// Macro to increment the SP
#define INCREMENT_SP(ammount) ((cpu.registers[SP])=(cpu.registers[SP])+(ammount))
// Macro to decrement the SP
#define DECREMENT_SP(ammount) ((cpu.registers[SP])=(cpu.registers[SP])-(ammount))

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
        printf("%s : [%04x] %d \n",reg_str[i], cpu.registers[i], cpu.registers[i]);
    }
}

// Get the next word in memory
unsigned short get_next_bytecode()
{
    unsigned short bytecode=0;
    unsigned char lower = memory[GET_PC()];
    unsigned char upper = memory[GET_PC()+1];
    bytecode=(bytecode|upper)<<8;
    bytecode|=lower;
    return bytecode;
}

int push_stack(unsigned short value)
{
    memory[GET_SP()]=value;
    DECREMENT_SP(WORD_SIZE);
}
unsigned short pop_stack()
{
    unsigned char value = memory[GET_SP()];
    INCREMENT_SP(WORD_SIZE);
    return value;
}
unsigned short peek_stack()
{
    unsigned char value = memory[GET_SP()];
    return value;
}

// Either return an immediate value, or a pointer for an immediate location
int get_immediate_value(int code, unsigned short ** immediate, int pointer_flag)
{
    // registers
    if (code>=0 && code <=16) { if(pointer_flag) { *immediate=&cpu.registers[code];}else{ **immediate=cpu.registers[code];}}
    // If we are setting a push, set the immediate destination to the next sp value
    else if(code==17) if(pointer_flag) {DECREMENT_SP(WORD_SIZE); *immediate=(unsigned short *)&(memory[cpu.registers[SP]]); } else { **immediate = 0; }
    // POP
    else if(code==18) {if(!pointer_flag) { **immediate=pop_stack();}}
    // PEEK
    else if(code==19) {if(!pointer_flag) { **immediate=peek_stack();}}
    // Check if the xy value is specifying we are using an extended-word immediate
    else if(code>=20 && code<=25) 
    {
        // Get the next bytecode value
        INCREMENT_PC(WORD_SIZE);
        unsigned short imm=get_next_bytecode();
        if(code==20 || code == 21) **immediate=imm;
        else if(code==23) if(pointer_flag) { *immediate=(unsigned short *)&memory[imm]; } else{ **immediate=memory[imm]; }
        else if(code==24) if(pointer_flag) { *immediate=(unsigned short *)&memory[cpu.registers[imm]]; } else{ **immediate=memory[cpu.registers[imm]]; }
        else if(code==25)
        {
            if(imm==18) if(pointer_flag) { *immediate=(unsigned short *)&memory[pop_stack()]; } else{ **immediate=memory[pop_stack()]; }
            else if(imm==19) if(pointer_flag) { *immediate=(unsigned short *)&memory[peek_stack()]; } else{ **immediate=memory[peek_stack()]; }
        }
    }
    return 1;
}



// Functions for opcode operations
int res_op(unsigned short ** x, unsigned short **y){return 0;}
int nop_op(unsigned short ** x, unsigned short **y){return 0;}
int set_op(unsigned short ** x, unsigned short **y){**x=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int add_op(unsigned short ** x, unsigned short **y){**x+=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int sub_op(unsigned short ** x, unsigned short **y){**x-=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int mul_op(unsigned short ** x, unsigned short **y){**x*=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int div_op(unsigned short ** x, unsigned short **y){**x/=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int and_op(unsigned short ** x, unsigned short **y){**x=**x&**y;INCREMENT_PC(WORD_SIZE);return 0;}
int or_op(unsigned short ** x, unsigned short **y) {**x=**x|**y;INCREMENT_PC(WORD_SIZE);return 0;}
int xor_op(unsigned short ** x, unsigned short **y){**x=**x^**y;INCREMENT_PC(WORD_SIZE);return 0;}
int not_op(unsigned short ** x, unsigned short **y){**x=~(**x);INCREMENT_PC(WORD_SIZE);return 0;}
int mod_op(unsigned short ** x, unsigned short **y){**x%=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int rem_op(unsigned short ** x, unsigned short **y){**x/=**y;INCREMENT_PC(WORD_SIZE);return 0;}
int srl_op(unsigned short ** x, unsigned short **y){**x=**x>>**y;INCREMENT_PC(WORD_SIZE);return 0;}
int sll_op(unsigned short ** x, unsigned short **y){**x=**x<<**y;INCREMENT_PC(WORD_SIZE);return 0;}
int sra_op(unsigned short ** x, unsigned short **y){**x=**x>>**y;INCREMENT_PC(WORD_SIZE);return 0;}
int sla_op(unsigned short ** x, unsigned short **y){**x=**x<<**y;INCREMENT_PC(WORD_SIZE);return 0;}
int ieq_op(unsigned short ** x, unsigned short **y)
{
    if(**x == **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int ine_op(unsigned short ** x, unsigned short **y)
{
    if(**x != **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int ige_op(unsigned short ** x, unsigned short **y)
{
    if(**x >= **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int igt_op(unsigned short ** x, unsigned short **y)
{
    if(**x > **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int ilt_op(unsigned short ** x, unsigned short **y)
{
    if(**x < **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int ile_op(unsigned short ** x, unsigned short **y)
{
    if(**x <= **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int ibs_op(unsigned short ** x, unsigned short **y)
{
    // Skip over the next word if x isn't greater than or equal to y
    if(**x && **y) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int inb_op(unsigned short ** x, unsigned short **y)
{
    // Skip over the next word if x isn't greater than or equal to y
    if(!(**x && **y)) INCREMENT_PC(WORD_SIZE);
    else INCREMENT_PC(WORD_SIZE*2);
    return 0;
}
int jmp_op(unsigned short ** x, unsigned short **y)
{
    cpu.registers[PC]=**x;
    return 0;
}
int jal_op(unsigned short ** x, unsigned short **y)
{
    push_stack(GET_PC());
    cpu.registers[PC]=**x;
    return 0;
}
int rtn_op(unsigned short ** x, unsigned short **y)
{
    cpu.registers[PC]=pop_stack();
    return 0;
}
int sys_op(unsigned short ** x, unsigned short **y) {return 0;}
int int_op(unsigned short ** x, unsigned short **y) {return 0;}

// Pointers for opcode functions
int (*opcode_functions[MAX_OP])(unsigned short ** x, unsigned short ** y) = 
{
    res_op, nop_op, set_op, add_op, sub_op, mul_op, div_op, and_op, or_op, xor_op, 
    not_op, mod_op, rem_op, srl_op, sll_op, sra_op, sla_op, ieq_op, ine_op, ige_op, 
    igt_op, ilt_op, ile_op, ibs_op, inb_op, jmp_op, jal_op, rtn_op, sys_op, int_op
};

// Execute the emulator program
int execute_prog()
{
    // Run the FDE cycle
    while(1)
    {
        // Ensure we cannot go out of bounds in memory
        if(GET_PC()<0 || GET_PC()>MEM_SIZE) break;
        unsigned short bytecode=get_next_bytecode();
        // Get the opcode and the x and y values
        unsigned char op=bytecode&0x3F,x=(bytecode>>6)&0x1F,y=(bytecode>>11)&0x1F;
        unsigned short source_immediate=0,dest_immediate=0;
        // x is always a destination, and never a value
        unsigned short * source_pointer=&source_immediate;
        unsigned short * dest_pointer=&dest_immediate;
        // Check if we need an immediate value for x and y, and if so set it
        get_immediate_value(x, &source_pointer, 1);
        get_immediate_value(y, &dest_pointer,0);
        if(op==NIL || bytecode==NIL) break;
        // Call the relevant opcode function
        (*opcode_functions[op]) (&source_pointer, &dest_pointer);
        // Check if the ir register is non 0
        if(cpu.registers[IR]) printf("Interrupt: %x\n", cpu.registers[IR]);
    }
    print_registers();
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
    unsigned char previous_op=NIL;
    // Currently, we have to loop over every bytecode
    // This is becuase a 0x0 could be an integer value, and not NOP operation :(
    while(1)
    {
        // Get the lower 8 bits of the bytecode and write it to the next memory address
        unsigned char lower = (bytecode[i])&0xFF;
        // Check if we have reached a NIL instruction
        if(previous_op==NIL && lower==NIL) break;
        memory[memory_counter]=lower;
        memory_counter+=1;
        // printf("Written to memory location %x: %x\n",memory_counter,lower);
        // Get the upper 8 bits of the bytecode and write it to the next memory address
        unsigned char upper = ((bytecode[i])>>8);
        memory[memory_counter]=upper;
        memory_counter+=1;
        // Marking the previous operation for break checking
        previous_op=lower;
        i++;
    }

    // Initialise CPU registers
    cpu=(Cpu){ .registers={0} };
    cpu.registers[PC]=TEXT_ADDR;
    cpu.registers[SP]=STACK_ADDR;
    return 0;
}

int load_prog(char * name)
{   
    // Load the file and write the bytecode to an unsigned short array
    FILE * read_file;
    read_file=fopen(name,"rb");
    fread(&bytecode,MAX_PROG_LEN,sizeof(unsigned short),read_file);
    return 0;
}

// Get the extension of a filename
const char * get_filename_ext(const char * filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

// Validate the file has the correct extension
int validate_filename_ext(const char * filename)
{
    if(strcmp(get_filename_ext(filename),BINARY_EXTENSION)==0) return 1;
    return -1;
}

int main(int argc, char *argv[])
{
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;}
    if(setup_emulator()<0) {printf("Failed to setup emulator!\n");return -1;}
    if(execute_prog()<0) {printf("Failed to execute program!\n");return -1;}
    return 0;
}