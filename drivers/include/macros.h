//
// Created by cout970 on 14/04/18.
//

#ifndef COMPUTER_MACROS_H
#define COMPUTER_MACROS_H

#ifndef MAX
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

// Equivalent to 'ceil( (float) x / (float) y)' but without using floats
#define CEIL_DIV(x, y) (((x) > 0)? 1 + ((x) - 1)/(y): ((x) / (y)))


#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#ifndef SIZE_MAX
#define SIZE_MAX INT_MAX
#endif


#endif //COMPUTER_MACROS_H
