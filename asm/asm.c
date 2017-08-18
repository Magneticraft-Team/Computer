//
// Created by cout970 on 2016-12-16.
//

#include "asm.h"
#include "read.h"

enum Tokens {
    NO_TOKEN,
    ERROR,
    ADD,
    ADDI,
    ADDIU,
    ADDU,
    AND,
    ANDI,
    BEQ,
    BGEZ,
    BGEZAL,
    BGTZ,
    BLEZ,
    BLTZ,
    BLTZAL,
    BNE,
    DIV,
    DIVU,
    J,
    JAL,
    JR,
    LB,
    LUI,
    LW,
    MFHI,
    MFLO,
    MULT,
    MULTU,
    NOOP,
    OR,
    ORI,
    SB,
    SLL,
    SLLV,
    SLT,
    SLTI,
    SLTIU,
    SLTU,
    SRA,
    SRL,
    SRLV,
    SUB,
    SUBU,
    SW,
    SYSCALL,
    XOR,
    XORI,
    NAME,
    NUMBER,
    DOLAR,
    DOT,
    LF,
    SEMICOLON,
    COMA,
    LPAREN,
    RPAREN
};

const char *keywords[] = {
        "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ", "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR",
        "XOR",
        NULL
};

const char *tokenNames[] = {
        "No token", "Error", "ADD", "ADDI", "ADDIU", "ADDU", "AND", "ANDI", "BEQ", "BGEZ", "BGEZAL", "BGTZ", "BLEZ",
        "BLTZ", "BLTZAL", "BNE",
        "DIV", "DIVU", "J", "JAL", "JR", "LB", "LUI", "LW", "MFHI", "MFLO", "MULT", "MULTU", "NOOP", "OR", "ORI", "SB",
        "SLL", "SLLV", "SLT", "SLTI", "SLTIU", "SLTU", "SRA", "SRL", "SRLV", "SUB", "SUBU", "SW", "SYSCALL", "XOR",
        "XOR",
        "NAME", "NUMBER", "$", ".", "\\n", ":", ",", "(", ")"
};

const char *registerNames[] = {
        "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1",
        "s2",
        "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};


#define input_buffer_size 120
char inputBuffer[input_buffer_size];
int inputIndex;
int charLook;

char tokenBuffer[input_buffer_size];
int tokenLook;

jmp_buf onError;

//compilation output buffer
int codeBufferIndex = 0;
uint8_t codeBuffer[1024];

typedef struct {
    char identifier[32];
    int index;
} IdentifierItem;

int identifierTableIndex = 0;
IdentifierItem identifierTable[128];

void readChar() {

    if (inputIndex >= input_buffer_size || inputBuffer[inputIndex] == '\0') {
        charLook = -1;
    }
    charLook = inputBuffer[inputIndex++];
}

int stringDiff(const char *const a, const char *const b) {
    int i = 0;
    while (1) {
        if (toupper(a[i]) != toupper(b[i])) return 1;
        if (a[i] == '\0') break;
        i++;
    }
    return 0;
}

int scanToken() {
    int p = 0;

    if (charLook == -1) {
        return NO_TOKEN;
    }

    //trim
    while (charLook == ' ' || charLook == '\t') {
        readChar();
        if (charLook == -1) {
            return NO_TOKEN;
        }
    }

    //name or keyword
    if (isalpha(charLook)) {
        do {
            tokenBuffer[p++] = (char) charLook;
            readChar();

            if (charLook == -1) {
                break;
            }
        } while (isalnum(charLook));

        tokenBuffer[p] = '\0';

        for (int i = 0; keywords[i] != NULL; i++) {
            if (!stringDiff(keywords[i], tokenBuffer)) {
                return i + 2;
            }
        }
        return NAME;

        //number
    } else if (isdigit(charLook)) {

        do {
            tokenBuffer[p++] = (char) charLook;
            readChar();

            if (charLook == -1) {
                break;
            }
        } while (isdigit(charLook));

        tokenBuffer[p] = '\0';
        return NUMBER;
    } else {
        switch (charLook) {
            case '$':
                readChar();
                return DOLAR;
            case '.':
                readChar();
                return DOT;
            case ':':
                readChar();
                return SEMICOLON;
            case ',':
                readChar();
                return COMA;
            case '(':
                readChar();
                return LPAREN;
            case ')':
                readChar();
                return RPAREN;
            case '#':
                while (charLook != '\n' && charLook != -1) {
                    readChar();
                }
                return scanToken();
            case '\0':
                return NO_TOKEN;
            default:
                break;
        }
    }
    printf("Invalid token: \'%c\' %d\n", (char) charLook, charLook);
    return ERROR;
}

void printToken(int token) {
    printf("Token: %d, (%s) ", token, tokenNames[token]);
    if (token == NAME || token == NUMBER) {
        printf("'%s'", tokenBuffer);
    }
    printf("\n");
}

void readToken() {
    tokenLook = scanToken();
}

void write(unsigned int code) {
    ((int *) codeBuffer)[codeBufferIndex++] = code;
}

void need(int tk) {
    if (tokenLook != tk) {
#ifdef DEBUG
        printf("look: %d, expected: %d\n", tokenLook, tk);
#endif
        if (tk == NO_TOKEN) {
            printf("Expected input end, but found \'%s\'\n", tokenNames[tokenLook]);
        } else if (tokenLook < 0) {
            printf("Expected \'%s\', but found input end\n", tokenNames[tk]);
        } else if (tokenLook == NAME) {// || token_look == STRING || token_look == CHAR
            printf("Expected (%s), but found (%s) \'%s\'\n", tokenNames[tk], tokenNames[tokenLook], tokenBuffer);
        } else {
            printf("Expected (%s), but found (%s)\n", tokenNames[tk], tokenNames[tokenLook]);
        }
        longjmp(onError, 1);
    }
}

void expect(int tk) {
    readToken();
    need(tk);
}

int parseRegister() {
    int num = -1;
    if (tokenLook == NUMBER) {
        num = atoi(tokenBuffer);
        if (num < 0 || num > 32) {
            printf("Unknown token trying to parse a register: \n");
            printToken(tokenLook);
            longjmp(onError, 1);
        }
    } else if (tokenLook == NAME) {
        for (int i = 0; i < 32; i++) {
            if (!stringDiff(registerNames[i], tokenBuffer)) {
                num = i;
                break;
            }
        }
        if (num == -1) {
            printf("Unknown token trying to parse a register: \n");
            printToken(tokenLook);
            longjmp(onError, 1);
        }
    } else {
        printf("Unknown token trying to parse a register: \n");
        printToken(tokenLook);
        longjmp(onError, 1);
    }
    return num;
}

int parseInt() {
    if (tokenLook == NUMBER) {
        return atoi(tokenBuffer);
    } else {
        printf("Unknown token trying to parse an integer: \n");
        printToken(tokenLook);
        longjmp(onError, 1);
    }
}

void compileRInstruction(int reg1, int reg2, int reg3, int sht, int func) {

    /* R type:
     * -------------------------------------------
     * | oooo ooss ssst tttt dddd dhhh hhcc cccc |
     * -------------------------------------------
     *   0 <---------------------------------> 32
     */
    unsigned int instr = 0U;
    instr |= (reg1 & 0b11111) << 21;
    instr |= (reg2 & 0b11111) << 16;
    instr |= (reg3 & 0b11111) << 11;
    instr |= (sht & 0b11111) << 6;
    instr |= (func & 0b111111);
#ifdef DEBUG
    printf("0x%08x (%d)\n", instr, instr);
    printf("%d $%d, $%d, $%d sht: %d\n", func, reg1, reg2, reg3, sht);
#endif
    write(instr);
}

void compileIInstruction(int opcode, int reg1, int reg2, int inmed) {

    /* I type:
     * -------------------------------------------
     * | oooo ooss ssst tttt iiii iiii iiii iiii |
     * -------------------------------------------
     *   0 <---------------------------------> 32
     */
    unsigned int instr = 0U;
    instr |= (opcode & 0b111111) << 26;
    instr |= (reg1 & 0b11111) << 21;
    instr |= (reg2 & 0b11111) << 16;
    instr |= inmed & 0xFFFF;
#ifdef DEBUG
    printf("0x%08x (%d)\n", instr, instr);
    printf("%d $%d, $%d, %d  (0x%08x)\n", opcode, reg1, reg2, inmed, inmed);
#endif
    write(instr);
}

void compileJInstruction(int opcode, int addr) {

    /* J type:
     * -------------------------------------------
     * | oooo ooii iiii iiii iiii iiii iiii iiii |
     * -------------------------------------------
     *   0 <---------------------------------> 32
     */
    unsigned int instr = 0U;
    instr |= (opcode & 0b111111) << 26;
    instr |= ((addr >> 2) & 0x3FFFFFF);

#ifdef DEBUG
    printf("0x%08x (%d)\n", instr, instr);
    printf("opcode = %d, addr = %d (0x%x)\n", opcode, addr, addr);
#endif
    write(instr);
}

void parse(int type, int code) {
    int reg1, reg2, reg3, offset, addr = -1, i;
    switch (type) {
        case 0:// R: name $r1, $r2, $r3
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(COMA);
            expect(DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(COMA);
            expect(DOLAR);
            readToken();
            reg3 = parseRegister();
            compileRInstruction(reg2, reg3, reg1, 0, code);
            break;
        case 1:// I: name $s, $t, imm
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(COMA);
            expect(DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(COMA);
            expect(NUMBER);
            compileIInstruction(code, reg1, reg2, atoi(tokenBuffer));
            break;
        case 2:// R: name $s, $t, imm
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(COMA);
            expect(DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(COMA);
            expect(NUMBER);
            compileRInstruction(reg1, reg2, 0, atoi(tokenBuffer), code);
            break;
        case 3:// R: name $s, $t
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(COMA);
            expect(DOLAR);
            readToken();
            reg2 = parseRegister();
            compileRInstruction(reg1, reg2, 0, 0, code);
            break;
        case 4:// R: name $t, offset($s)
            expect(DOLAR);
            readToken();
            reg2 = parseRegister();
            expect(COMA);
            readToken();
            offset = parseInt();
            expect(LPAREN);
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(RPAREN);
            compileIInstruction(code, reg1, reg2, offset);
            break;
        case 5: // R: name location
            expect(NAME);
            for (i = 0; i < identifierTableIndex; ++i) {
                if (!stringDiff(identifierTable[i].identifier, tokenBuffer)) {
                    addr = identifierTable[i].index * 4;
                    break;
                }
            }
            if (i == identifierTableIndex) {
                printf("Unknown point to jump (%s)\n", tokenBuffer);
                printf("Valid points: ");
                for (i = 0; i < identifierTableIndex; ++i) {
                    printf("%s ", identifierTable[i].identifier);
                }
                putchar('\n');
                longjmp(onError, 1);

            } else if (addr == -1) {
                printf("Invalid pointer to jump %x (%s)\n", addr, tokenBuffer);
                longjmp(onError, 1);
            }
#ifdef DEBUG
            printf("addr 0x%08x, name: %s\n", addr, identifierTable[i].identifier);
#endif
            compileJInstruction(code, addr);
            break;
        case 7: // R: name $s, imm
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(COMA);
            readToken();
            offset = parseInt();
            compileIInstruction(code, 0, reg1, offset);
            break;
        case 8: // R: name $s
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            compileRInstruction(reg1, 0, 0, 0, code);
            break;
        case 9: // R: name $s
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            compileRInstruction(0, 0, reg1, 0, code);
            break;
        default:
            break;
    }
}

void parseVariant(int type, int code, int subCode) {
    int reg1, offset;
    switch (type) {
        case 6: // R: name $s, imm
            expect(DOLAR);
            readToken();
            reg1 = parseRegister();
            expect(COMA);
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
        case MFLO:
            parse(9, 0b010010);
            break;
        case MFHI:
            parse(9, 0b010000);
            break;
        case JR:
            parse(8, 0b001000);
            break;
        case LUI:
            parse(7, 0b001111);
            break;
        case BLTZAL:
            parseVariant(6, 0b000001, 0b10000);
            break;
        case BLTZ:
            parseVariant(6, 0b000001, 0b00000);
            break;
        case BLEZ:
            parseVariant(6, 0b000110, 0b00000);
            break;
        case BGTZ:
            parseVariant(6, 0b000111, 0b00000);
            break;
        case BGEZAL:
            parseVariant(6, 0b000001, 0b10001);
            break;
        case BGEZ:
            parseVariant(6, 0b000001, 0b00001);
            break;
        case J:
            parse(5, 0b000010);
            break;
        case JAL:
            parse(5, 0b000011);
            break;
        case SB:
            parse(4, 0b101000);
            break;
        case SW:
            parse(4, 0b101011);
            break;
        case LB:
            parse(4, 0b100000);
            break;
        case LW:
            parse(4, 0b100011);
            break;
        case DIV :
            parse(3, 0b011010);
            break;
        case DIVU:
            parse(3, 0b011011);
            break;
        case MULT:
            parse(3, 0b011000);
            break;
        case MULTU:
            parse(3, 0b011001);
            break;
        case ADDI://addi
            parse(1, 0b001000);
            break;
        case ADDIU://addi
            parse(1, 0b001001);
            break;
        case ANDI://andi
            parse(1, 0b001100);
            break;
        case BEQ://beq
            parse(1, 0b000100);
            break;
        case BNE://bne
            parse(1, 0b000101);
            break;
        case ORI://ori
            parse(1, 0b001101);
            break;
        case SLL://sll
            parse(2, 0b000000);
            break;
        case SLTI://slti
            parse(1, 0b001010);
            break;
        case SLTIU://slti
            parse(1, 0b001011);
            break;
        case SRA://sra
            parse(2, 0b000011);
            break;
        case SRL://srl
            parse(2, 0b000010);
            break;
        case XORI://xori
            parse(1, 0b001110);
            break;
        case ADD: //add 2 reg
            parse(0, 0b100000);
            break;
        case ADDU://add 2 reg
            parse(0, 0b100001);
            break;
        case AND://logical and 2 reg
            parse(0, 0b100100);
            break;
        case OR://logical or 2 reg
            parse(0, 0b100101);
            break;
        case SLLV://shift left logical variable
            parse(0, 0b000100);
            break;
        case SLT://set on less than (signed)
            parse(0, 0b101010);
            break;
        case SLTU://set on less than unsigned
            parse(0, 0b101011);
            break;
        case SRLV://shift right logical variable
            parse(0, 0b000110);
            break;
        case SUB://subtract 2 reg
            parse(0, 0b100010);
            break;
        case SUBU://subtract unsigned 2 reg
            parse(0, 0b100011);
            break;
        case XOR://bitwise exclusive or
            parse(0, 0b100110);
            break;
        case NOOP:
            write(0b001100);
            break;
        case SYSCALL:
            write(0b001100);
        case NAME:
            expect(SEMICOLON);

            item = &(identifierTable[identifierTableIndex]);
            strcpy(item->identifier, tokenBuffer);
            item->index = codeBufferIndex;
#ifdef DEBUG
            printf("name = '%s', addr = %d\n", tokenBuffer, codeBufferIndex);
#endif
            identifierTableIndex++;
            break;
        case DOT:
            expect(NAME);
            break;
        case NO_TOKEN:
            return;
        case ERROR:
            longjmp(onError, 1);
        default:
            printf("Unexpected token: ");
            printToken(tokenLook);
    }
}

int loopTick() {

    readLine(inputBuffer, input_buffer_size);

    if (!stringDiff(inputBuffer, ":exit")) {
        return 1;
    }

    //reset
    inputIndex = 0;
    readChar();

    if (!setjmp(onError)) {
        do {
            readToken();
            parseLine();
        } while (tokenLook != NO_TOKEN && tokenLook != ERROR);
    } else { //error
        printf("Error, ignoring last line\n");
    }
    return 0;
}

void printCode() {
    printf("\n");
    for (int i = 0; i < codeBufferIndex; ++i) {
        printf("0x%08x\n", ((int *) codeBuffer)[i]);
    }
}
