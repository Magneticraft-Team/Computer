//
// Created by cout970 on 19/01/18.
//
// Basic output utilities, by default prints to screen, with USE_DEBUG_LOG prints to the minecraft log
//

#ifndef COMPUTER_KPRINT_H
#define COMPUTER_KPRINT_H

#include "types.h"

// Print char to the stdout
void kputchar(Char c);

// Print Signed number to the stdout,
// returns the number of characters printed
Int kprintn(Int num, Int base);

// Print unsigned number to the stdout,
// returns the number of characters printed
Int kprintun(UInt num, Int base);

// Print a string to the stdout,
// returns the number of characters printed
Int kprints(const String *str);

// Print 'count' characters of the string 'str' to stdout,
// returns the number of characters printed
Int kprintsn(const String *str, Int count);

// Basic printf functionality, supports %(d, i, u, x, o, b, c, s),
// returns the number of characters printed
Int kprint(const String *str, ...);

#endif //COMPUTER_KPRINT_H
