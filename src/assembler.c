/* 
James Clarke 20/05/19 
assembler.c
This file takes a .narl file as an input and assembles it to a binary
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_ADDR 0x2000
#define MAX_REG 16
#define MAX_PROG_LEN 256
#define MAX_LINE_LEN 128
#define MAX_OP 28
#define MAX_XY_FUNCS 3

// Macro to convert a line number to a text address position
#define LINE_TO_ADDR(line) (TEXT_ADDR+((line)*(2)))
// Macro to convert a text address position to a line number
#define ADDR_TO_LINE(addr) (((addr)-(TEXT_ADDR))/2)

typedef struct {
    unsigned short * array;
    size_t used;
    size_t size;
} BytecodeArray;

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
// Number of lines
int prog_len;
// Copy of the original program, used for macro searching
char ** original_prog;
// The assembled binaries
BytecodeArray bytecode_array;

// Initialise a bytecode array to be written to disk
void init_bytecode_array(BytecodeArray * a, size_t initial)
{
    a->array=(unsigned short*)malloc(sizeof(unsigned short)*initial);
    a->used=0;
    a->size=initial;
}

// Dynamically insert an element to the array
void insert_bytecode_array(BytecodeArray * a, unsigned short element)
{
    if(a->used==a->size)
    {
        a->size+=1;
        a->array=(unsigned short*)realloc(a->array, a->size * sizeof(unsigned short));
    }
    a->array[a->used++]=element;
}

// Free the bytecode array
void free_bytecode_array(BytecodeArray * a)
{
    free(a->array);
    a->array=NULL;
    a->used=a->size = 0;
}

// Remove a substring from a string
char * str_remove(char *str, const char *sub) 
{
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        size_t size = 0;
        while ((p = strstr(p, sub)) != NULL) {
            size = (size == 0) ? (p - str) + strlen(p + len) + 1 : size - len;
            memmove(p, p + len, size - (p - str));
        }
    }
    return str;
}

// Write the bytecode array to disk
int write_bytecode(char * name)
{
    // Remove the .narl extention and add the .narlb extension
    name = str_remove(name, ".narl");
    strcat(name, ".narlb");
    // Open the file in binary write mode
    FILE * write_file;
    write_file=fopen(name,"wb");
    // Write the bytecode array to the file
    fwrite(bytecode_array.array, sizeof(short), sizeof(bytecode_array.array), write_file);
    // Close the file and free the bytecode
    fclose(write_file);
    free_bytecode_array(&bytecode_array);
    return 0;
}

// Check if an opcode string exists and return it's index
int check_op_index(char * op)
{
    int i;
    // check all opcodes for a string match
    for(i=0;i<MAX_OP;i++)
    {
        if(strcmp(op, opcodes[i])==0) return i;
    }
    return 0;
}

// Check if an xy atomic value is a register
int check_reg_index(char * str){
	int i;
    // check all registers for a string match
	for(i=0;i<MAX_REG;i++)
    {
		if(!strcmp(str,reg_str[i])) return i;
	}
	return 0;
} 

// Check if an xy atomic value is a stack operation function
int check_is_stackfunc(char * str)
{
    int i;
    for (i=0;i<MAX_XY_FUNCS;i++)
    {
        // If the str matches any stack function, return 17 + the index
        if(strcmp(str, xy_funcs[i])==0) return 17+i;
    }
    return 0;
}

// Check if an xy atomic value is a signed integer
int check_is_int(char * str)
{
    int i;
    // Check if each integer isn't a digit, if so it's not an integer
    for(i=0;i<strlen(str);i++) if(isdigit(str[i])==0) return 0;
    return 1;
}

// Check if an xy atomic value is an unsigned integer
int check_is_uint(char * str)
{
    // If the string is prefixed with a 'u' and the rest is an integer
    if(str[0]=='u' && check_is_int(++str)) return 1;
    return 0;
}

// Check if an xy atomic value is a memory address
int check_is_mem(char * str)
{
    // First check if memory specifier is correct format
    if(str[0]=='[' && str[strlen(str)-1]==']')
    {
        // Get the string without the bracket characters
        char * mem_num=str;
        mem_num++;
        mem_num[strlen(mem_num)-1]=0;
        // Check whether the address is a number, register value, or stack function
        if(check_is_int(mem_num) != -1) return 1;
        if(check_reg_index(mem_num) != -1) return 2;
        if(check_is_stackfunc(mem_num) != -1) return 3;
    }
    return 0;
}

int check_is_label(char * str)
{
    char lbl[MAX_LINE_LEN]={0};
    char * str_cpy=strdup(str);
    strcat(lbl,"# ");
    strcat(lbl,str_cpy);
    lbl[strlen(lbl)]='\0';
    // Loop over every line and see if it contains a label
    int i=0;
    int macro_counter=0;
    while(i<prog_len)
    {
        // We have found the label as a substring to the line
        if(strstr(original_prog[i], lbl) != NULL)
        {
            return LINE_TO_ADDR(i-macro_counter);
        }
        // If we are on a macro line, we want to skip it as it doesn't count as an address line
        if(strstr(original_prog[i], "#") != NULL) macro_counter++;
        i++;
    }
    free(str_cpy);
    return 0;
}

int check_is_dat(char * str)
{
    int i=0;
    char * dat_line;
    while(i<prog_len)
    {
        // Check if we are on a data macro line
        if(strstr(original_prog[i], "#") != NULL && strstr(original_prog[i], "DAT") != NULL)
        {
            // If we have found the macro we are looking for
            if(strstr(original_prog[i], str) != NULL)
            {
                // Copy the line to a string that we can process
                dat_line=strdup(original_prog[i]);
                dat_line[strlen(dat_line)]='\0';
                char * next = strtok(dat_line," ");
                int word_count=0;
                // Else lex standard statement
                while(next != NULL)
                {
                    // Check to see if we are on the value, return it as an integer
                    if(word_count == 3) return atoi(next);
                    next = strtok(NULL," ");  
                    word_count++;
                }
                free(dat_line);
            }
        }
        i++;
    }
    return 0;
}

// Get the x/y value for a given string, and assign the optional extended word values
int get_xy_val(char * xy, short * s, unsigned short * us, int * s_flag, int * us_flag)
{
    // Check if xy is in registers
    int reg = check_reg_index(xy);
    if(reg) return reg;

    // Check if xy is in xy_funcs
    int stack_func = check_is_stackfunc(xy);
    if (stack_func) return stack_func;

    // check if xy is a signed int
    if(check_is_int(xy)) 
    {
        *s=atoi(xy);
        *s_flag=1;
        return 20;
    }

    // check if xy is an unsigned int
    if(check_is_uint(xy)) 
    {
        *us=atoi(++xy);
        *us_flag=1;
        return 21;
    }

    // check if xy is a floating point value
    // UNSUPPORTED CURRENTLY
    // char *ptr;
    // if(strtod(xy,&ptr)!=0) return 22;

    // check if xy is a memory address
    int mem=check_is_mem(xy);
    if(mem!=-1)
    {
        // Remove the brackets from the address string
        char * mem_addr=xy;
        mem_addr++;
        mem_addr[strlen(mem_addr)]=0;
        // Check what type of address specifier we have (immediate, register, stack operation function)
        switch(mem)
        {
            case 1: {*s = atoi(mem_addr); *s_flag=1; return 23; }
            case 2: {*s = check_reg_index(mem_addr); *s_flag=1; return 24; } 
            case 3: {*s = check_is_stackfunc(mem_addr); *s_flag=1; return 25; } 
        }
    }

    // check if xy is a data macro
    int dat = check_is_dat(xy);
    if(dat) 
    {
        *s=dat;
        *s_flag=1; 
        return 20;
    }

    // check if xy is a label address
    int label=check_is_label(xy);
    if(label) 
    {
        *us=label;
        *us_flag=1; 
        return 23;
    }

    return -1;
}

// Generate bytecode integers from the text
int make_bytecode()
{
    // Initialise the bytecode array
    init_bytecode_array(&bytecode_array,1);

    // Loop through the program
	int line_counter;
	for(line_counter=0;line_counter<prog_len;line_counter++)
	{
        // Check if the line is empty
        if(prog[line_counter][0] == '\0') continue;
        // Bytecode to be modified
        unsigned short bytecode=0;
        // The word we are in on the raw string
        int counter=0;
        // Get the next word
        char * next = strtok(prog[line_counter]," ,\n");

        // Check if we are lexing macro
        if(strcmp(next,"#")==0) continue;

        // for either x or y, they may need an extra word for an immediate value
        int sx_flag=0, usx_flag=0, sy_flag=0, usy_flag=0;
        short sx=0, sy =0;
        unsigned short usx=0, usy=0;
        // Else lex standard statement
        while(next != NULL)
        {
            int temp_s_flag=0, temp_us_flag=0;
            short temp_s=0;
            unsigned short temp_us=0;

            // Get the value that should be inserted into the oxy position
            int val = (counter==0) ? check_op_index(next) : get_xy_val(next,&temp_s,&temp_us,&temp_s_flag,&temp_us_flag);
            if(val<0) return -1;
            // Get the bit position in the bytecode that it should be inserted into
            int bit_position = (counter==0) ? 0 : 6+(5*(counter-1));
            // Set the bytecode bits
            bytecode |= (val << bit_position);
            // Set the optional extended word immediate values
            if(temp_s_flag){if(counter==1){sx=temp_s;sx_flag=1;}else if(counter==2){sy=temp_s;sy_flag=1;}}
            if(temp_us_flag){if(counter==1){usx=temp_us;usx_flag=1;}else if(counter==2){usy=temp_us;usy_flag=1;}}
            // Get the next word & increase the counter
            next = strtok(NULL, " ,\n");
            counter++;
        }
        // Insert the bytecode to the bytecode array
        insert_bytecode_array(&bytecode_array,bytecode);
        // Insert optional multi-word immediates
        if(sx_flag)insert_bytecode_array(&bytecode_array,sx);
        else if(usx_flag)insert_bytecode_array(&bytecode_array,usx);
        if(sy_flag)insert_bytecode_array(&bytecode_array,sy);
        else if(usy_flag)insert_bytecode_array(&bytecode_array,usy);
	}
    // Insert a NOP opeartor to the end
    insert_bytecode_array(&bytecode_array,(unsigned short) 0x0);
    return 0;
}

// Load the program line by line into an array of lines
int load_prog(char* prog_name)
{
    original_prog=malloc(MAX_PROG_LEN * sizeof(char*));
	FILE * file = NULL;
	file = fopen(prog_name,"r");

	if(file==NULL) return -1;

	int line_counter=0;
	// Loop through each file line and add it to the prog array
	while(fgets(prog[line_counter],MAX_LINE_LEN,file)!=NULL)
	{
        if(prog[line_counter][0]=='\n') break;
		// Add a null terminator to the line string
        // for some reason the below line causes the bytecode maker not to work :(
		prog[line_counter][strlen(prog[line_counter])]='\0';
        original_prog[line_counter]=strdup(prog[line_counter]);
		line_counter++;
        
	}
    prog_len=line_counter;
	int i;

    return 0;
}

int main(int argc, char *argv[])
{
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;};
    if(make_bytecode()<0) {printf("Failed to assemble!\n");return -1;};
    if(write_bytecode(argv[1])<0) {printf("Failed to write to disk!\n");return -1;}
    return 0;
}