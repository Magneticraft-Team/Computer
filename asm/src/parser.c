//
// Created by cout970 on 17/02/18.
//

#include <debug.h>
#include "../include/parser.h"
#include "../include/asm.h"
#include "../include/lexer.h"
#include "../include/reader.h"
#include "../include/code_generator.h"

const char *tokenNames[] = {
        "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ",
        "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR",
        "XOR", "NAME", "NUMBER", "$", ".", "\\n", ":", ",", "(", ")", "EOF", "Error"
};

const char *registerNames[] = {
        "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1",
        "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

// TODO remove
typedef struct {
    char identifier[32];
    int index;
} IdentifierItem;

int identifierTableIndex = 0;
IdentifierItem identifierTable[128];

void printToken(int token) {
    kdebug("Token: %d, (%s) ", token, tokenNames[token]);
    if (token == TK_NAME || token == TK_NUMBER) {
        kdebug("'%s'", tokenBuffer);
    }
    kdebug("\n");
}

int parseInt() {
    if (tokenLook == TK_NUMBER) {
        int len = strlen(tokenBuffer);
        char *end = NULL;
        int value = strtol(tokenBuffer, &end, tokenNumberBase);
        if (end != (tokenBuffer + len)) {
            kdebug("[%d, %d] Error trying to parse integer: '%s'\n", currentLine, currentColumn, tokenBuffer);
            longjmp(onError, 1);
        }
        return value;
    } else {
        kdebug("[%d, %d] Unknown token trying to parse an integer: \n", currentLine, currentColumn);
        printToken(tokenLook);
        longjmp(onError, 1);
    }
}

int parseRegister() {
    int num = -1;
    if (tokenLook == TK_NUMBER) {
        num = atoi(tokenBuffer);
        if (num < 0 || num > 32) {
            kdebug("[%d, %d] Unknown token trying to parse a register: \n", currentLine, currentColumn);
            printToken(tokenLook);
            longjmp(onError, 1);
        }
    } else if (tokenLook == TK_NAME) {
        for (int i = 0; i < 32; i++) {
            if (strcmp(registerNames[i], tokenBuffer) == 0) {
                num = i;
                break;
            }
        }
        if (num == -1) {
            kdebug("[%d, %d] Unknown token trying to parse a register: \n", currentLine, currentColumn);
            printToken(tokenLook);
            longjmp(onError, 1);
        }
    } else {
        kdebug("[%d, %d] Unknown token trying to parse a register: \n", currentLine, currentColumn);
        printToken(tokenLook);
        longjmp(onError, 1);
    }
    return num;
}

void parse(int type, int code) {
    int reg1, reg2, reg3, offset, addr = -1, i, aux;
    switch (type) {
        case 0:// R: name $r1, $r2, $r3
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            expect(TK_DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(TK_COMA);
            expect(TK_DOLAR);
            readToken();
            reg3 = parseRegister();
            compileRInstruction(reg2, reg3, reg1, 0, code);
            break;
        case 1:// I: name $t, $s, imm
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            expect(TK_DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(TK_COMA);
            expect(TK_NUMBER);
            aux = parseInt();
            compileIInstruction(code, reg2, reg1, aux);
            break;
        case 2:// R: name $d, $t, imm
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            expect(TK_DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(TK_COMA);
            expect(TK_NUMBER);
            compileRInstruction(reg1, reg2, 0, parseInt(), code);
            break;
        case 3:// R: name $s, $t
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            expect(TK_DOLAR);
            readToken();
            reg2 = parseRegister();
            compileRInstruction(reg1, reg2, 0, 0, code);
            break;
        case 4:// R: name $t, offset($s)
            expect(TK_DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(TK_COMA);
            readToken();
            offset = parseInt();
            expect(TK_LPAREN);
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_RPAREN);
            compileIInstruction(code, reg1, reg2, offset);
            break;
        case 5: // R: name location
            expect(TK_NAME);
            for (i = 0; i < identifierTableIndex; ++i) {
                if (strcmp(identifierTable[i].identifier, tokenBuffer) == 0) {
                    addr = identifierTable[i].index * 4;
                    break;
                }
            }
            if (i == identifierTableIndex) {
                kdebug("[%d, %d] Unknown point to jump (%s)\n", currentLine, currentColumn, tokenBuffer);
                kdebug("Valid points: ");
                for (i = 0; i < identifierTableIndex; ++i) {
                    kdebug("%s ", identifierTable[i].identifier);
                }
                putchar('\n');
                longjmp(onError, 1);

            } else if (addr == -1) {
                kdebug("[%d, %d] Invalid pointer to jump %x (%s)\n", currentLine, currentColumn, addr, tokenBuffer);
                longjmp(onError, 1);
            }
#ifdef DEBUG
            kdebug("addr 0x%08x, name: %s\n", addr, identifierTable[i].identifier);
#endif
            compileJInstruction(code, addr);
            break;
        case 7: // R: name $s, imm
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            readToken();
            offset = parseInt();
            compileIInstruction(code, 0, reg1, offset);
            break;
        case 8: // R: name $s
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            compileRInstruction(reg1, 0, 0, 0, code);
            break;
        case 9: // R: name $s
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            compileRInstruction(0, 0, reg1, 0, code);
            break;
        case 10:// I: name $s, $t, imm
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            expect(TK_DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(TK_COMA);
            expect(TK_NUMBER);
            aux = parseInt();
            compileIInstruction(code, reg1, reg2, aux);
            break;
        default:
            break;
    }
}

void parseVariant(int type, int code, int subCode) {
    int reg1, offset;
    switch (type) {
        case 6: // R: name $s, imm
            expect(TK_DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(TK_COMA);
            readToken();
            offset = parseInt();
            compileIInstruction(code, reg1, subCode, offset);
            break;
        default:
            break;
    }
}

void parseLine() {
    IdentifierItem *item = NULL;
    switch (tokenLook) {
        case TK_MFLO:
            parse(9, 0b010010);
            break;
        case TK_MFHI:
            parse(9, 0b010000);
            break;
        case TK_JR:
            parse(8, 0b001000);
            break;
        case TK_LUI:
            parse(7, 0b001111);
            break;
        case TK_BLTZAL:
            parseVariant(6, 0b000001, 0b10000);
            break;
        case TK_BLTZ:
            parseVariant(6, 0b000001, 0b00000);
            break;
        case TK_BLEZ:
            parseVariant(6, 0b000110, 0b00000);
            break;
        case TK_BGTZ:
            parseVariant(6, 0b000111, 0b00000);
            break;
        case TK_BGEZAL:
            parseVariant(6, 0b000001, 0b10001);
            break;
        case TK_BGEZ:
            parseVariant(6, 0b000001, 0b00001);
            break;
        case TK_J:
            parse(5, 0b000010);
            break;
        case TK_JAL:
            parse(5, 0b000011);
            break;
        case TK_SB:
            parse(4, 0b101000);
            break;
        case TK_SW:
            parse(4, 0b101011);
            break;
        case TK_LB:
            parse(4, 0b100000);
            break;
        case TK_LW:
            parse(4, 0b100011);
            break;
        case TK_DIV :
            parse(3, 0b011010);
            break;
        case TK_DIVU:
            parse(3, 0b011011);
            break;
        case TK_MULT:
            parse(3, 0b011000);
            break;
        case TK_MULTU:
            parse(3, 0b011001);
            break;
        case TK_ADDI://addi
            parse(1, 0b001000);
            break;
        case TK_ADDIU://addi
            parse(1, 0b001001);
            break;
        case TK_ANDI://andi
            parse(1, 0b001100);
            break;
        case TK_BEQ://beq
            parse(10, 0b000100);
            break;
        case TK_BNE://bne
            parse(10, 0b000101);
            break;
        case TK_ORI://ori
            parse(1, 0b001101);
            break;
        case TK_SLL://sll
            parse(2, 0b000000);
            break;
        case TK_SLTI://slti
            parse(1, 0b001010);
            break;
        case TK_SLTIU://slti
            parse(1, 0b001011);
            break;
        case TK_SRA://sra
            parse(2, 0b000011);
            break;
        case TK_SRL://srl
            parse(2, 0b000010);
            break;
        case TK_XORI://xori
            parse(1, 0b001110);
            break;
        case TK_ADD: //add 2 reg
            parse(0, 0b100000);
            break;
        case TK_ADDU://add 2 reg
            parse(0, 0b100001);
            break;
        case TK_AND://logical and 2 reg
            parse(0, 0b100100);
            break;
        case TK_OR://logical or 2 reg
            parse(0, 0b100101);
            break;
        case TK_SLLV://shift left logical variable
            parse(0, 0b000100);
            break;
        case TK_SLT://set on less than (signed)
            parse(0, 0b101010);
            break;
        case TK_SLTU://set on less than unsigned
            parse(0, 0b101011);
            break;
        case TK_SRLV://shift right logical variable
            parse(0, 0b000110);
            break;
        case TK_SUB://subtract 2 reg
            parse(0, 0b100010);
            break;
        case TK_SUBU://subtract unsigned 2 reg
            parse(0, 0b100011);
            break;
        case TK_XOR://bitwise exclusive or
            parse(0, 0b100110);
            break;
        case TK_NOOP:
            writeInstruction(0b000000);
            break;
        case TK_SYSCALL:
            writeInstruction(0b001100);
        case TK_NAME:
            expect(TK_SEMICOLON);

            item = &(identifierTable[identifierTableIndex]);
            strcpy(item->identifier, tokenBuffer);
            item->index = codeBufferIndex;
#ifdef DEBUG
            kdebug("name = '%s', addr = %d\n", tokenBuffer, codeBufferIndex);
#endif
            identifierTableIndex++;
            break;
        case TK_DOT:
            expect(TK_NAME);
            break;
        case TK_EOF:
            return;
        case TK_ERROR:
            longjmp(onError, 1);
        default:
            kdebug("[%d, %d] Unexpected token: ", currentLine, currentColumn);
            printToken(tokenLook);
    }
}