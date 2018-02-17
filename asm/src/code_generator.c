//
// Created by cout970 on 17/02/18.
//

#include <fs/file.h>
#include "../include/code_generator.h"

#define CODE_BUFFER_SIZE 1024

//compilation output buffer
int codeBufferIndex = 0;
char codeBuffer[CODE_BUFFER_SIZE];
FD dstFile;

void initCodeGenerator(FD dst) {
    codeBufferIndex = 0;
    dstFile = dst;
}

void writeInstruction(unsigned int code) {
    ((int *) codeBuffer)[codeBufferIndex++] = code;
    if (codeBufferIndex == CODE_BUFFER_SIZE) {
        file_write(dstFile, (ByteBuffer const) codeBuffer, CODE_BUFFER_SIZE);
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
    kdebug("0x%08x (%d)\n", instr, instr);
    kdebug("%d $%d, $%d, $%d sht: %d\n", func, reg1, reg2, reg3, sht);
#endif
    writeInstruction(instr);
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
    kdebug("0x%08x (%d)\n", instr, instr);
    kdebug("%d $%d, $%d, %d  (0x%08x)\n", opcode, reg1, reg2, inmed, inmed);
#endif
    writeInstruction(instr);
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
    kdebug("0x%08x (%d)\n", instr, instr);
    kdebug("opcode = %d, addr = %d (0x%x)\n", opcode, addr, addr);
#endif
    writeInstruction(instr);
}