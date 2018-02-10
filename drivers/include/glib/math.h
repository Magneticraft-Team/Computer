//
// Created by cout970 on 2017-08-07.
//

#ifndef COMPUTER_MATH_H
#define COMPUTER_MATH_H

#ifndef MAX
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

// Equivalent to 'ceil( (float) x / (float) y)' but without using floats
#define CEIL_DIV(x, y) (((x) > 0)? 1 + ((x) - 1)/(y): ((x) / (y)))

// The CPU doesn't support floating point instructions, if you want them talk to the developers

#endif //COMPUTER_MATH_H
