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

// Strings corresponding to indexes in the registers
const char *reg_str[] = {"pc", "sp", "ia", "cr", "r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"};
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
	return(-1);
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
		prog[line_counter][strlen(prog[line_counter])-1]='\0';
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

int make_bytecode()
{
	int line_counter;
	for(line_counter=0;line_counter<MAX_PROG_LEN;line_counter++)
	{
        short byte=0;
		int counter=0;
        char * next = strtok(prog[line_counter]," ,");
        while(next != NULL)
        {
            printf("%s\n",next);
            next = strtok(NULL, " ,");
        }
        getchar();
	}
    return 0;
}

int main(int argc, char *argv[])
{
    load_prog(argv[1]);
    make_bytecode();
    return 0;
}