/* 
James Clarke 20/05/19 
assembler.c
This file takes a .narl file as an input and assembles it to a binary
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "narlcodes.h"

#define COMMENT_CHAR ';'
#define DATA_MACRO "dat"

typedef struct {
    unsigned short * array;
    size_t used;
    size_t size;
} BytecodeArray;

// The program string
char prog[MAX_PROG_LEN][MAX_LINE_LEN];
// Record how many words each line index contains
int num_words[MAX_PROG_LEN];
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
    fwrite(bytecode_array.array, sizeof(unsigned short), bytecode_array.used, write_file);
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
    return -1;
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
    int flag=1;
    // Check if each integer isn't a digit, if so it's not an integer
    for(i=0;i<strlen(str);i++) if(isdigit(str[i])==0) flag = 0;
    // If we have a valid integer, return it
    if (flag) return atoi(str);
    // Check if it is in hex format
    char * str_cpy=strdup(str);
    if (str_cpy[strspn(str_cpy, "0x123456789abcdefABCDEF")] == 0) return (int)strtol(str_cpy, NULL, 16);
    // Else not a number [return -1 as 0 is a valid number]
    return -1;
}

// Check if an xy atomic value is an unsigned integer
int check_is_uint(char * str)
{
    int check=check_is_int(++str);
    // If the string is prefixed with a 'u' and the rest is an integer
    // Not the pointer has to be decremented as we previously incremented it to get the check val
    if((--str)[0]=='u' && check) return check;
    // Else not a number [return -1 as 0 is a valid number]
    return -1;
}

// Check if an xy atomic value is a memory address
int check_is_mem(char * str)
{
    // First check if memory specifier is correct format
    if(str[0]=='[' && str[strlen(str)-1]==']')
    {
        // Get the string without the bracket characters
        char * mem_num=(++str);
        mem_num[strlen(mem_num)-1]='\0';
        // Check whether the address is a number, register value, or stack function
        if(check_is_int(mem_num)>=0) return 1;
        if(check_reg_index(mem_num)) return 2;
        if(check_is_stackfunc(mem_num)) return 3;
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
    int i=0, macro_counter=0;
    while(i<prog_len)
    {
        // We have found the label as a substring to the line
        if(strstr(original_prog[i], lbl) != NULL && strchr(original_prog[i], '#') != NULL)
        {   
            return LINE_TO_ADDR(i-macro_counter);
        }
        // // If we are on a macro line, we want to skip it as it doesn't count as an address line
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
        if(strstr(original_prog[i], "#") != NULL && strstr(original_prog[i], DATA_MACRO) != NULL)
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
        return 20;
    }

    // Check if xy is in registers
    int reg = check_reg_index(xy);
    if(reg) return reg;

    // Check if xy is in xy_funcs
    int stack_func = check_is_stackfunc(xy);
    if (stack_func) return stack_func;

    // check if xy is a signed int
    int num = check_is_int(xy);
    // Check > 0 as zero is a valid number
    if(num >= 0) 
    {
        *s=num;
        *s_flag=1;
        return 20;
    }
    // check if xy is an unsigned int
    int unum = check_is_uint(xy);
    // Check > 0 as zero is a valid number
    if(unum >= 0) 
    {
        *us=unum;
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
            case 1: {*s = check_is_int(mem_addr); *s_flag=1; return 23; break;}
            case 2: {*s = check_reg_index(mem_addr); *s_flag=1; return 24; break;} 
            case 3: {*s = check_is_stackfunc(mem_addr); *s_flag=1; return 25; break;} 
        }
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
        if(prog[line_counter][0] == '\0') {num_words[line_counter]=0;continue;};
        // Bytecode to be modified
        unsigned short bytecode=0;
        // The word we are in on the raw string
        int counter=0;
        // Get the next word
        char * next = strtok(prog[line_counter]," ,\n");

        // Check if we are lexing macro
        if(strcmp(next,"#")==0) {num_words[line_counter]=0;continue;};

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
        // Record how many words this line takes
        num_words[line_counter]=1;
        // Insert the bytecode to the bytecode array
        insert_bytecode_array(&bytecode_array,bytecode);
        int word_counter=1;
        // Insert optional multi-word immediates
        if(sx_flag){insert_bytecode_array(&bytecode_array,sx); word_counter++;}
        else if(usx_flag){insert_bytecode_array(&bytecode_array,usx); word_counter++;}
        if(sy_flag){insert_bytecode_array(&bytecode_array,sy); word_counter++;}
        else if(usy_flag){insert_bytecode_array(&bytecode_array,usy); word_counter++;}
        // Insert a bytecode value of 0 for extra
        while(word_counter<3)
        { 
            insert_bytecode_array(&bytecode_array,(unsigned short)0x0);
            word_counter++;
        }
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
        //if(prog[line_counter][0]=='\n') break;
        if(prog[line_counter][0]=='\n') continue;
        // Remove comments from the line
        char *ptr;
        ptr = strchr(prog[line_counter], COMMENT_CHAR);
        if (ptr != NULL) {
            *ptr = '\0';
        }
		// Add a null terminator to the line string
		prog[line_counter][strlen(prog[line_counter])]='\0';
        original_prog[line_counter]=strdup(prog[line_counter]);
		line_counter++;
        
	}
    prog_len=line_counter;
	int i;

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
    if(strcmp(get_filename_ext(filename),SOURCE_EXTENSION)==0) return 1;
    return -1;
}

int main(int argc, char *argv[])
{
    if(validate_filename_ext(argv[1])<0) {printf("Invalid filename!\n");return -1;}
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;};
    if(make_bytecode()<0) {printf("Failed to assemble!\n");return -1;};
    if(write_bytecode(argv[1])<0) {printf("Failed to write to disk!\n");return -1;}
    return 0;
}