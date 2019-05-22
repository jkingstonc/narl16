/* 
James Clarke 22/05/19 
narlcodes.h
Relevant information about the ISA
*/

#ifndef NARL_CODES_H
#define NARL_CODES_H

// Base address for memory locations
#define TEXT_ADDR 0x2000
#define STATIC_ADDR 0x6000
#define HEAP_ADDR 0x7000
#define STACK_ADDR 0xE000
#define VIDEO_ADDR 0xF000

#define MAX_PROG_LEN 256
#define MAX_LINE_LEN 128

#define MAX_OP 28
#define MAX_XY_FUNCS 3
#define MAX_REG 16

// Strings corresponding to indexes of opcodes
const char *opcodes[] = 
    {"NOP","SET","ADD","SUB","MUL","DIV","AND","OR","XOR","NOT",
     "MOD", "REM", "SRL", "SLL", "SRA", "SLA", "IEQ", "INE", "IGE", 
     "IGT", "ILT", "ILE", "IBS", "INB", "JAL", "RTN", "SYS", "INT"};
// Strings corresponding to indexes in the registers
const char *reg_str[] = {"pc", "sp", "ia", "cr", "r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"};
// Strings corresponding to xy available stack functions
const char *xy_funcs[] = {"PSH","POP","PEEK"};

#endif