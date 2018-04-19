
#ifndef LISP_READER_H
#define LISP_READER_H

#include <fs/file.h>

struct ReaderState;

void rd_init();

struct ReaderState* rd_setInput(FD file);

void rd_recover(struct ReaderState *oldState);

int rd_readChar();

void rd_unreadChar();

#endif //LISP_READER_H
