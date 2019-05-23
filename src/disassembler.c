/* 
James Clarke 20/05/19 
assembler.c
This file takes an assembled .narl file [narl binary] as an input and outputs the dissasembled assembly code
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "narlcodes.h"

unsigned short bytecode[MAX_PROG_LEN];

char * get_op_str(char op_index)
{
    char * str = strdup(opcodes[op_index]);
    return str;
}

char * get_xy_str(char val, char ** str_ptr, int * line_counter)
{
    // Check if the value is a register, if so copy the register string constant
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
    else if(val==20) {(*line_counter)++;sprintf(*str_ptr, "%d", bytecode[*line_counter]);}
    // Check if the value is an unsigned immediate
    else if(val==21) {(*line_counter)++;sprintf(*str_ptr, "%hu", bytecode[*line_counter]);}

    else if(val==23) {
        (*line_counter)++;
        char new_str[MAX_LINE_LEN];
        new_str[0]='[';
        char new_num[MAX_LINE_LEN];
        sprintf(new_num, "%d", bytecode[*line_counter]);
        strcat(new_str, new_num);
        strcat(new_str, "]");
        *str_ptr=new_str;
    }
    else if(val==24) {
        (*line_counter)++;
        char * new_str=(char*)malloc(sizeof(char));
        strcpy(new_str, "[");
        char * new_num=strdup(reg_str[bytecode[*line_counter]]);
        strcat(new_str, new_num);
        strcat(new_str, "]");
        *str_ptr=new_str;
    }
    else{
        *str_ptr="";
    }
}

int dissasemble_prog()
{
    unsigned short next_word;
    int line_counter=0;
    while(bytecode[line_counter]!=0x0)
    {
        char op = (bytecode[line_counter])&0x3F;
        char x = (bytecode[line_counter]>>6)&0x1F;
        char y = (bytecode[line_counter]>>11)&0x1F;
        printf("x: %d y: %d\n",x,y);
        // Initialise strings that will be printed
        char * op_str=get_op_str(op), *x_str=malloc(sizeof(char)*7), *y_str=malloc(sizeof(char)*7);
        // Get the strings for the x and y value
        get_xy_str(x,&x_str, &line_counter);
        get_xy_str(y,&y_str, &line_counter);
        printf("%s %s %s\n",get_op_str(op),x_str,y_str);
        line_counter++;
    }
    return 0;
}

int load_prog(char * name)
{   
    // Load the file and write the bytecode to an unsigned short array
    FILE * read_file;
    read_file=fopen(name,"rb");
    fread(&bytecode,8,sizeof(unsigned short),read_file);
    return 0;
}

int main(int argc, char *argv[])
{
    if(load_prog(argv[1])<0){printf("Failed to load program!\n");return -1;}
    if(dissasemble_prog()<0) {printf("Failed to dissasemble program!\n");return -1;}
    return 0;
}