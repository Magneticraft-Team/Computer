//
// Created by cout970 on 17/02/18.
//

#ifndef COMPUTER_CODE_GENERATOR_H
#define COMPUTER_CODE_GENERATOR_H

#include <fs/file.h>

void initCodeGenerator(FD dst);

void writeRawInstruction(unsigned int rawInstruction);

void compileRInstruction(int reg1, int reg2, int reg3, int sht, int func);

void compileIInstruction(int opcode, int reg1, int reg2, int inmed);

void compileJInstruction(int opcode, int addr);

#endif //COMPUTER_CODE_GENERATOR_H
