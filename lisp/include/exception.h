//
// Created by cout970 on 09/03/18.
//

#ifndef COMPUTER_EXCEPTION_H
#define COMPUTER_EXCEPTION_H

#include <glib/setjmp.h>

jmp_buf tryBuffer;
int catchCode;

#define EXCEPTION_UNBOUND_SYMBOL 1
#define EXCEPTION_NOT_A_FUNCTION 2
#define EXCEPTION_INVALID_TOKEN 3
#define EXCEPTION_TOKEN_BUFFER_OVERFLOW 4
#define EXCEPTION_DIVIDED_BY_ZERO 5
#define EXCEPTION_CAST_TO_NUMBER_FAILED 6
#define EXCEPTION_OUT_OF_MEMORY 7
#define EXCEPTION_NOT_A_SYMBOL 8

#define TRY if(!(catchCode = setjmp(tryBuffer)))
#define CATCH else
#define THROW(x) longjmp(tryBuffer, x)

#endif //COMPUTER_EXCEPTION_H
