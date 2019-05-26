/* 
James Clarke 22/05/19 
narlcodes.h
Relevant information about the ISA
*/

#ifndef NARL_CODES_H
#define NARL_CODES_H

#define SOURCE_EXTENSION "narl"
#define BINARY_EXTENSION "narlb"

// Word size in bytes
#define WORD_SIZE 2
// Instruction size in bytes
#define INSTRUCTION_SIZE 6

// Base address for memory locations
#define VIDEO_ADDR 0xF000
#define STACK_ADDR 0xE000
#define HEAP_ADDR 0x8000
#define STATIC_ADDR 0x7000
#define TEXT_ADDR 0x4000
#define OS_ADDR 0x0
#define MEM_SIZE 0x10000

#define MAX_PROG_LEN 1024
#define MAX_LINE_LEN 128

#define MAX_OP 30
#define MAX_XY_FUNCS 3
#define MAX_REG 16

#define NIL 0

#define PC 0
#define SP 1
#define IR 2
#define CR 3

// Macro to convert a line number to a text address position
#define LINE_TO_ADDR(line) ((TEXT_ADDR)+((line)*(INSTRUCTION_SIZE)))

// Strings corresponding to indexes of opcodes
const char *opcodes[] = 
    {"NIL", "NOP","SET","ADD","SUB","MUL","DIV","AND","OR","XOR","NOT",
     "MOD", "REM", "SRL", "SLL", "SRA", "SLA", "IEQ", "INE", "IGE", 
     "IGT", "ILT", "ILE", "IBS", "INB", "JMP", "JAL", "RTN", "SYS", "INT"};
// Strings corresponding to indexes in the registers
const char *reg_str[] = {"pc", "sp", "ir", "cr", "r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12"};
// Strings corresponding to xy available stack functions
const char *xy_funcs[] = {"PSH","POP","PEK"};

#endif