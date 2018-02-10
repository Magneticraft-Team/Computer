//
// Created by cout970 on 2016-11-01.
//
// Allows to jump between pieces of code, can be used to implement a primitive try/catch

#ifndef COMPUTER_SETJMP_H
#define COMPUTER_SETJMP_H

/**
 * Buffer for registers
 */
typedef int jmp_buf[31];

/**
 * This function stores in buff the current environment of the program, basically registers, to return when longjmp is called
 * Returns 0 if the function is called, and non zero value if longjmp is called, in fact returns the
 * value of the argument of longjmp
 */

int setjmp(jmp_buf buf);

/**
 * Returns to the last setjmp call and act as if setjmp() had returned
 * 'value' (which better be non-zero!).
 */
void longjmp(jmp_buf buf, int value);

#endif //COMPUTER_SETJMP_H
