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

unsigned short get_next_bytecode()
{
    unsigned short bytecode=0;
    unsigned char lower = memory[GET_PC()];
    // INCREMENT_PC(1);
    unsigned char upper = memory[GET_PC()+1];
    // INCREMENT_PC(1);
    bytecode=(bytecode|upper)<<8;
    bytecode|=lower;
    return bytecode;
}

int push_stack(unsigned short value)
{
    memory[GET_SP()]=value;
    // Stack pointer deals in words not bytes [1 word = 2 bytes]
    DECREMENT_SP(2);
}
unsigned short pop_stack()
{
    unsigned char value = memory[GET_SP()];
    // Stack pointer deals in words not bytes [1 word = 2 bytes]
    INCREMENT_SP(2);
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
    // XY value that doesn't need another extended word
    if(code>=0 && code <=19)
    {
        switch(code)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14: 
            case 15:
            case 16:
            {
                if(pointer_flag) { *immediate=&cpu.registers[code];}else{ **immediate=cpu.registers[code];}
                break;
            } // Registers
            case 17:
            {
                // If we are setting a push, set the immediate destination to the next sp value
                if(pointer_flag) {DECREMENT_SP(2); *immediate=(unsigned short *)&(memory[cpu.registers[SP]]); } else { **immediate = 0; }
                break;
            } // Push
            case 18:
            {
                if(pointer_flag) {}else{ **immediate=pop_stack(); }
                break;
            } // Pop
            case 19:
            {
                if(pointer_flag) {}else{ **immediate=peek_stack();}
                break;
            } // Peek
        }
    }
    // Check if the xy value is specifying we are using an extended-word immediate
    else if(code>=20 && code<=25) 
    {
        // Get the next bytecode value
        INCREMENT_PC(2);
        unsigned short imm=get_next_bytecode();
        switch(code)
        {
            case 20: 
            {   
                **immediate=imm;
                break; 
            } // Signed immediate
            case 21: 
            {
                **immediate=imm;
                break; 
            } // Unsigned immediate
            case 22: break; // Half prescision IEE 754 fp
            case 23: 
            {
                if(pointer_flag) { *immediate=(unsigned short *)&memory[imm]; } else{ **immediate=memory[imm]; }
                break;
            } // Memory immediate
            case 24:
            {
                if(pointer_flag) { *immediate=(unsigned short *)&memory[cpu.registers[imm]]; } else{ **immediate=memory[cpu.registers[imm]]; }
                break;
            } // Memory register contents
            case 25:
            {
                switch(imm)
                {
                    case 17: 
                    {
                        
                        break; 
                    } // PSH
                    case 18: 
                    {
                        if(pointer_flag) { *immediate=(unsigned short *)&memory[pop_stack()]; } else{ **immediate=memory[pop_stack()]; }
                        break; 
                    } // POP
                    case 19: 
                    {
                        if(pointer_flag) { *immediate=(unsigned short *)&memory[peek_stack()]; } else{ **immediate=memory[peek_stack()]; }
                        break;
                    } // PEK
                }
            } // Memory stack contents
        }
    }
    return 1;
}

int execute_prog()
{
    int running =1;
    // Run the FDE cycle
    while(running && GET_PC()<MEM_SIZE)
    {
        unsigned short bytecode=get_next_bytecode();
        if(bytecode==0x1) running=0;
        // Get the opcode and the x and y values
        unsigned char op=bytecode&0x3F,x=(bytecode>>6)&0x1F,y=(bytecode>>11)&0x1F;
        unsigned short source_immediate=0,dest_immediate=0;
        // x is always a destination, and never a value
        unsigned short * source_pointer=&source_immediate;
        unsigned short * dest_pointer=&dest_immediate;

        // Check if we need an immediate value for x and y, and if so set it
        get_immediate_value(x, &source_pointer, 1);
        get_immediate_value(y, &dest_pointer,0);

        switch(op)
        {
            case 0x0: {running=0;break;}
            case 0x2: // SET
            {
                *source_pointer=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x3: // ADD
            {
                *source_pointer+=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x4: // SUB
            {
                *source_pointer-=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x5: // MUL
            {
                *source_pointer*=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x6: // DIV
            {
                *source_pointer/=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x7: // AND
            {
                *source_pointer&=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x8: // OR
            {
                *source_pointer|=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x9: // XOR
            {
                *source_pointer^=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0xA: // NOT
            {
                *source_pointer=~*(source_pointer);
                INCREMENT_PC(2);
                break;
            }
            case 0xB: // MOD
            {
                *source_pointer%=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0xC: // REM
            {
                *source_pointer/=*dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0xD: // SRL
            {
                *source_pointer=*source_pointer >> *dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0xE: // SLL
            {
                *source_pointer=*source_pointer << *dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0xF: // SRA
            {
                *source_pointer=*source_pointer >> *dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x10: // SLA
            {
                *source_pointer=*source_pointer << *dest_pointer;
                INCREMENT_PC(2);
                break;
            }
            case 0x11: // IEQ
            {
                // Skip over the next word if the values aren't equal
                if(*source_pointer == *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x12: // INE
            {
                // Skip over the next word if the values are equal
                if(*source_pointer != *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x13: // IGE
            {
                // Skip over the next word if x isn't greater than or equal to y
                if(*source_pointer >= *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x14: // IGT
            {
                // Skip over the next word if x isn't greater than y
                if(*source_pointer > *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x15: // ILT
            {
                // Skip over the next word if x isn't less than y
                if(*source_pointer < *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x16: // ILE
            {
                // Skip over the next word if x isn't less than or equal to y
                if(*source_pointer <= *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x17: // IBS
            {
                // Skip over the next word if x and y arent set
                if(*source_pointer && *dest_pointer) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x18: // INB
            {
                // Skip over the next word if x and y arent set
                if(!(*source_pointer && *dest_pointer)) INCREMENT_PC(2);
                else INCREMENT_PC(4);
                break;
            }
            case 0x19: // JMP
            {
                printf("jumping to: %x\n",*source_pointer);
                // Jump to the destination
                cpu.registers[PC]=*source_pointer;
                break;
            }
            case 0x1A: // JAL
            {
                // Push the program counter to the stack and jump
                push_stack(GET_PC());
                cpu.registers[PC]=*source_pointer;
                break;
            }
            case 0x1B: // RTN
            {
                cpu.registers[PC]=pop_stack();
                break;
            }
            case 0x1C: // SYS
            {break;}
            case 0x1D: // INT
            {break;}
        }
        // Check if we have reached a NOP
        if(bytecode == 0x0) break;
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
    while(bytecode[i]!=0x0)
    {
        // Get the lower 8 bits of the bytecode and write it to the next memory address
        unsigned char lower = (bytecode[i])&0xFF;
        memory[memory_counter]=lower;
        memory_counter+=1;
        // printf("Written to memory location %x: %x\n",memory_counter,lower);
        // Get the upper 8 bits of the bytecode and write it to the next memory address
        unsigned char upper = ((bytecode[i])>>8);
        memory[memory_counter]=upper;
        memory_counter+=1;
        // printf("Written to memory location %x: %x\n",memory_counter,upper);
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

int main(int argc, char *argv[])
{
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;}
    if(setup_emulator()<0) {printf("Failed to setup emulator!\n");return -1;}
    if(execute_prog()<0) {printf("Failed to execute program!\n");return -1;}
    return 0;
}