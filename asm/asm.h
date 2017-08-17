//
// Created by cout970 on 2016-12-16.
//

#ifndef COMPUTER_ASM_H
#define COMPUTER_ASM_H

#include "dependencies.h"

enum Tokens {
    NO_TOKEN, ERROR, ADD, ADDI, ADDIU, ADDU, AND, ANDI, BEQ, BGEZ, BGEZAL, BGTZ, BLEZ, BLTZ, BLTZAL, BNE, DIV, DIVU, J, JAL, JR, LB, LUI,
    LW, MFHI, MFLO, MULT, MULTU, NOOP, OR, ORI, SB, SLL, SLLV, SLT, SLTI, SLTIU, SLTU, SRA, SRL, SRLV, SUB, SUBU, SW,
    SYSCALL, XOR, XORI, NAME, NUMBER, DOLAR, DOT, LF, SEMICOLON, COMA, LPAREN, RPAREN
};

const char *keywords[] = {
        "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ", "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR", "XOR",
        NULL
};

const char *tokenNames[] = {
        "No token", "Error", "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ", "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR", "XOR",
        "NAME", "NUMBER", "$", ".", "\\n", ":", ",", "(", ")"
};

const char *registerNames[] = {
        "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2",
        "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

int loopTick();

void printCode();

#endif //COMPUTER_ASM_H
