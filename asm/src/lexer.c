//
// Created by cout970 on 17/02/18.
//

#include <ctype.h>
#include <debug.h>
#include "../include/lexer.h"
#include "../include/reader.h"
#include "../include/asm.h"

char tokenBuffer[TOKEN_BUFFER_SIZE];
int tokenLook;
int tokenNumberBase = 10;

const char *tokenNames[] = {
        "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ",
        "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR",
        "XOR", "NAME", "NUMBER", "$", ".", "\\n", ":", ",", "(", ")", "EOF", "Error"
};

const char *keywords[] = {
        "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ", "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR",
        "XOR", NULL
};

int readNumber(int charLook) {
    int p = 0;
    char *validDigits = "0123456789";
    tokenNumberBase = 10;

    if (charLook == '0') {
        tokenNumberBase = 8;

        tokenBuffer[p++] = (char) charLook;
        charLook = readChar();

        if (charLook == 'x') {
            // x is not added to tokenBuffer
            p--; // remove the first 0

            //base 16, hexadecimal
            tokenNumberBase = 16;
            validDigits = "0123456789abcdefABCDEF";

            //make sure we are still reading a number
            charLook = readChar();
            if (charLook == -1 || !isxdigit(charLook)) {

                tokenBuffer[p] = '\0';
                unreadChar(charLook);
                return TK_ERROR;
            }
        } else {
            // base 8, octal
            tokenNumberBase = 8;
            validDigits = "01234567";

            if (charLook == -1 || strchr(validDigits, charLook) == NULL) {
                tokenBuffer[p] = '\0';
                unreadChar(charLook);
                return TK_NUMBER;
            }
        }
    }

    do {
        tokenBuffer[p++] = (char) charLook;
        charLook = readChar();
    } while (strchr(validDigits, charLook) != NULL);

    tokenBuffer[p] = '\0';
    unreadChar(charLook);
    return TK_NUMBER;
}

int scanToken() {
    int p = 0;
    int charLook = readChar();

    //trim
    while (isspace(charLook)) {
        charLook = readChar();
        if (charLook == -1) {
            return TK_EOF;
        }
    }

    if (charLook == '#') {
        while (charLook != '\n' && charLook != -1) {
            charLook = readChar();
        }
        //trim
        while (isspace(charLook)) {
            charLook = readChar();
            if (charLook == -1) {
                return TK_EOF;
            }
        }
    }

    //name or keyword
    if (isalpha(charLook)) {
        do {
            tokenBuffer[p++] = (char) charLook;
            charLook = readChar();
        } while (isalnum(charLook));

        unreadChar(charLook);
        tokenBuffer[p] = '\0';

        for (int i = 0; keywords[i] != NULL; i++) {
            if (strcmp(keywords[i], tokenBuffer) == 0) {
                return i;
            }
        }
        return TK_NAME;

        //number
    } else if (isdigit(charLook)) {
        return readNumber(charLook);
    } else {
        switch (charLook) {
            case '$':
                return TK_DOLAR;
            case '.':
                return TK_DOT;
            case ':':
                return TK_SEMICOLON;
            case ',':
                return TK_COMA;
            case '(':
                return TK_LPAREN;
            case ')':
                return TK_RPAREN;
            default:
                break;
        }
    }
    kdebug("[%d, %d] Invalid token: \'%c\' %d\n", currentLine, currentColumn, (char) charLook, charLook);
    return TK_ERROR;
}

void readToken() {
    tokenLook = scanToken();
}

void need(int tk) {
    if (tokenLook != tk) {
        if (tk == TK_EOF) {
            kdebug("[%d, %d] Expected input end, but found \'%s\'\n", currentLine, currentColumn,
                   tokenNames[tokenLook]);
        } else if (tokenLook < 0) {
            kdebug("[%d, %d] Expected \'%s\', but found input end\n", currentLine, currentColumn, tokenNames[tk]);
        } else if (tokenLook == TK_NAME) {// || token_look == STRING || token_look == CHAR
            kdebug("[%d, %d] Expected (%s), but found (%s) \'%s\'\n", currentLine, currentColumn, tokenNames[tk],
                   tokenNames[tokenLook], tokenBuffer);
        } else {
            kdebug("[%d, %d] Expected (%s), but found (%s)\n", currentLine, currentColumn, tokenNames[tk],
                   tokenNames[tokenLook]);
        }
        longjmp(onError, 1);
    }
}

void expect(int tk) {
    readToken();
    need(tk);
}