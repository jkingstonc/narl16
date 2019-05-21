/* 
James Clarke 20/05/19 
assembler.c
This file takes a .narl file as an input and assembles it to a binary
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TEXT_ADDR 0x2000
#define MAX_REG 12
#define MAX_PROG_LEN 256
#define MAX_LINE_LEN 128
#define MAX_OP 28
#define MAX_XY_FUNCS 3

// Strings corresponding to indexes of opcodes
const char *opcodes[] = 
    {"NOP","SET","ADD","SUB","MUL","DIV","AND","OR","XOR","NOT",
     "MOD", "REM", "SRL", "SLL", "SRA", "SLA", "IEQ", "INE", "IGE", 
     "IGT", "ILT", "ILE", "IBS", "INB", "JAL", "RTN", "SYS", "INT"};
// Strings corresponding to indexes in the registers
const char *reg_str[] = {"pc", "sp", "ia", "cr", "r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"};
// Strings corresponding to xy available stack functions
const char *xy_funcs[] = {"PSH","POP","PEEK"};

// The program string
char prog[MAX_PROG_LEN][MAX_LINE_LEN];
// The assembled binaries
short * prog_assembled;

// Get a register index from a register char... this should be a macro
int get_reg_index(char *reg){
	int i;
	for(i=0;i<MAX_REG;i++){
		if(!strcmp(reg,reg_str[i])){
			return(i);
		}
	}
	return -1;
} 

void remove_spaces(char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}

int load_prog(char* prog_name)
{
	FILE * file = NULL;
	file = fopen(prog_name,"r");

	if(file==NULL) return -1;

	int line_counter=0;
	// Loop through each file line and add it to the prog array
	while(fgets(prog[line_counter],MAX_LINE_LEN,file)!=NULL)
	{
		// Add a null terminator to the line string
        // for some reason the below line causes the bytecode maker not to work :(
		//prog[line_counter][strlen(prog[line_counter])-1]='\0';
		line_counter++;
	}
	int i;
	// print the program
	for(i=0;i<line_counter;i++)
	{
		printf("%s\n", prog[i]);
	}

    return 0;
}

int get_op_val(char * op)
{
    int i;
    for(i=0;i<MAX_OP;i++)
    {
        if(strcmp(op, opcodes[i])==0)
        {
            return i;
        }
    }
    return -1;
}

int get_xy_val(char * xy)
{
    // Check if xy is in registers
    if(get_reg_index(xy)!=-1) return get_reg_index(xy);
    // Check if xy is in xy_funcs
    int i;
    for (i=0;i<MAX_XY_FUNCS;i++)
    {
        if(strcmp(xy, xy_funcs[i])==0)
        {
            // PSH, POP, PEK encoding starts at 17
            return 17+i;
        }
    }
    return 0;
}

int make_bytecode()
{
    prog_assembled=malloc(sizeof(short)*MAX_PROG_LEN);
    int valid_line_counter=0;
	int line_counter;
	for(line_counter=0;line_counter<MAX_PROG_LEN;line_counter++)
	{
        // Check if the line is empty
        if(prog[line_counter][0] == '\0') continue;
        printf("line: %s\n",prog[line_counter]);
        unsigned short bytecode=0;
        int counter=0;
        char * next = strtok(prog[line_counter]," ,");

        // If we are lexing macro
        if(strcmp(next,"#")==0) {printf("Encountered macro...\n");continue;}
        // Else lex standard statement

        while(next != NULL)
        {
            // Get the value that should be inserted into the oxy position
            int val = (counter==0) ? get_op_val(next) : get_xy_val(next);
            // Get the bit position in the bytecode that it should be inserted into
            int bit_position = (counter==0) ? 0 : 6+(5*(counter-1));
            // Set the bytecode bits
            bytecode |= (val << bit_position);
            next = strtok(NULL, " ,");
            counter++;
        }
        printf("Bytecode: %x\n",bytecode);
	}
    return 0;
}

int main(int argc, char *argv[])
{
    load_prog(argv[1]);
    make_bytecode();
    return 0;
}