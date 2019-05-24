/* 
James Clarke 20/05/19 
assembler.c
This file takes an assembled .narl file [narl binary] as an input and outputs the dissasembled assembly code
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "narlcodes.h"

// The bytecode read from file
unsigned short bytecode[MAX_PROG_LEN];

// Convert an opcode to a string
char * get_op_str(char op_index)
{
    // Duplicate the constant string in the headder file
    char * str = strdup(opcodes[op_index]);
    return str;
}

// Convert an xy value to a string
char * get_xy_str(char val, char ** str_ptr, int * line_counter)
{
    // Check if the value is a register, if so copy the register string constant from the headder
    if(val>=0 && val<=16) *str_ptr=strdup(reg_str[val]);
    // Check if the value is a stack operation
    else if(val>=17 && val<=19) {
        switch(val)
        {
            case 17: *str_ptr = "PSH"; break;
            case 18: *str_ptr = "POP"; break;
            case 19: *str_ptr = "PEK"; break;
        }
    }
    // Check if the value is a signed immediate
    else if(val==20) {(*line_counter)++;sprintf(*str_ptr, "%x", bytecode[*line_counter]);}
    // Check if the value is an unsigned immediate
    else if(val==21) {(*line_counter)++;sprintf(*str_ptr, "%x", bytecode[*line_counter]);}
    // Check if the value is a memory immediate index
    else if(val==23) 
    {
        // Increase the counter to get the next word
        (*line_counter)++;
        // Copy the relevant formatting and the index to a new string
        char * new_str=(char*)malloc(sizeof(char)*1);
        strcpy(new_str, "[0x");
        char new_num[MAX_LINE_LEN];
        sprintf(new_num, "%x", bytecode[*line_counter]);
        strcat(new_str, new_num);
        strcat(new_str, "]");
        *str_ptr=new_str;
        //printf("sack\n");
    }
    // Check if the value is a memory register index
    else if(val==24) {
        // Increase the counter to get the next word
        (*line_counter)++;
        // Copy the relevant formatting, and get the register string to a new string
        char * new_str=(char*)malloc(sizeof(char));
        strcpy(new_str, "[");
        char * new_num=strdup(reg_str[bytecode[*line_counter]]);
        strcat(new_str, new_num);
        strcat(new_str, "]");
        *str_ptr=new_str;
    }
    // Check if the value is a memory stack operation index
    else if(val==25) {
        // Increase the counter to get the next word
        (*line_counter)++;
        // Copy the relevant formatting, and get the register string to a new string
        char * new_str=(char*)malloc(sizeof(char));
        strcpy(new_str, "[");
        char * new_num=strdup(xy_funcs[bytecode[*line_counter]-17]);
        strcat(new_str, new_num);
        strcat(new_str, "]");
        *str_ptr=new_str;
    }
    // Else the xy value is null
    else{
        *str_ptr="";
    }
}

// Dissassemble the program line by line
int dissasemble_prog()
{
    unsigned short next_word;
    int line_counter=0;
    unsigned char previous_op=NIL;
    while(line_counter<MAX_PROG_LEN)
    {
        // Get the bits for the opcode, x and y value
        char op = (bytecode[line_counter])&0x3F;
        if(previous_op==NIL && op==NIL) break;
        previous_op=op;
        if (op != 0x0) printf("[%x] ",LINE_TO_ADDR(line_counter));
        char x = (bytecode[line_counter]>>6)&0x1F;
        char y = (bytecode[line_counter]>>11)&0x1F;
        // Initialise strings that will be printed
        char * op_str=get_op_str(op), *x_str=malloc(sizeof(char)), *y_str=malloc(sizeof(char));
        // Get the strings for the x and y value
        get_xy_str(x,&x_str, &line_counter);
        get_xy_str(y,&y_str, &line_counter);
        if(previous_op!=NIL)printf("%s %s %s\n",get_op_str(op),x_str,y_str);
        //printf("%s %s %s\n",get_op_str(op),x_str,y_str);
        line_counter++;
    }
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
    if(validate_filename_ext(argv[1])<0) {printf("Invalid filename!\n");return -1;}
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;}
    if(dissasemble_prog()<0) {printf("Failed to dissasemble program!\n");return -1;}
    return 0;
}