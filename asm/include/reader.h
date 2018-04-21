//
// Created by cout970 on 17/02/18.
//

#ifndef COMPUTER_READER_H
#define COMPUTER_READER_H

#include <fs/file.h>

extern int currentLine;
extern int currentColumn;

void initReader(FD srcFile);

int rd_readChar();

void rd_unreadChar(int character);

#endif //COMPUTER_READER_H
