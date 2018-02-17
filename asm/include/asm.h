//
// Created by cout970 on 2016-12-16.
//

#ifndef COMPUTER_ASM_H
#define COMPUTER_ASM_H

#include "dependencies.h"

extern jmp_buf onError;

enum Tokens {
    TK_ADD,
    TK_ADDI,
    TK_ADDIU,
    TK_ADDU,
    TK_AND,
    TK_ANDI,
    TK_BEQ,
    TK_BGEZ,
    TK_BGEZAL,
    TK_BGTZ,
    TK_BLEZ,
    TK_BLTZ,
    TK_BLTZAL,
    TK_BNE,
    TK_DIV,
    TK_DIVU,
    TK_J,
    TK_JAL,
    TK_JR,
    TK_LB,
    TK_LUI,
    TK_LW,
    TK_MFHI,
    TK_MFLO,
    TK_MULT,
    TK_MULTU,
    TK_NOOP,
    TK_OR,
    TK_ORI,
    TK_SB,
    TK_SLL,
    TK_SLLV,
    TK_SLT,
    TK_SLTI,
    TK_SLTIU,
    TK_SLTU,
    TK_SRA,
    TK_SRL,
    TK_SRLV,
    TK_SUB,
    TK_SUBU,
    TK_SW,
    TK_SYSCALL,
    TK_XOR,
    TK_XORI,
    TK_NAME,
    TK_NUMBER,
    TK_DOLAR,
    TK_DOT,
    TK_LF,
    TK_SEMICOLON,
    TK_COMA,
    TK_LPAREN,
    TK_RPAREN,
    TK_EOF,
    TK_ERROR,
};

int f_compile(INodeRef src, INodeRef dst);

#endif //COMPUTER_ASM_H
