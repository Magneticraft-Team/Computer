//
// Created by cout970 on 2016-12-16.
//

#include <debug.h>
#include <fs/filesystem.h>
#include <fs/file.h>
#include "../include/asm.h"
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/code_generator.h"


jmp_buf onError;

int f_compile(FD src, FD dst) {

    initReader(src);
    initCodeGenerator(dst);

    if (!setjmp(onError)) {
        do {
            readToken();
            parseLine();
        } while (tokenLook != TK_EOF && tokenLook != TK_ERROR);
    } else { //error
        kdebug("Compile error\n");
        return 1;
    }
    return 0;
}