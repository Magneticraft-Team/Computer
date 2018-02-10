//
// Created by cout970 on 2016-11-01.
//
// This allows variable arguments, used for example in printf
//

#ifndef COMPUTER_STDARG_H
#define COMPUTER_STDARG_H

// List of variable arguments
typedef struct {
    int *_ptr;
    int _count;
} va_list;


// Initialization of the list, using the last argument of the function
#define va_start(list, arg) list._ptr = (int*)(&arg + 1); (list)._count = 0

// Retrieves a element from the list
#define va_arg(list, arg) ((arg)*((list)._ptr + (list)._count++))

// Removes the list, not needed in this implementation
#define va_end(list) ((void)0)

// Copies a va_list, so you can keep the original unchanged
#define va_copy(dst, src) dst._ptr = (src)._ptr; (dst)._count = (src)._count

// Example:
/*
void doStuff(const char *arg0, ...) {
    va_list list;
    va_start(list, arg0);

    int arg1 = va_arg(list, int);

    // do stuff with arg1...

    va_end(list);
}
*/

#endif //COMPUTER_STDARG_H
