//
// Created by cout970 on 2016-11-01.
//

#ifndef COMPUTER_STDARG_H
#define COMPUTER_STDARG_H

// List of variable arguments
#define va_list struct { int* _ptr; int _count; }

// Initialization of the list, using the last argument of the function
#define va_start(list, arg) list._ptr = (int*)(&arg + 1); (list)._count = 0

// Retrieves a element from the list
#define va_arg(list, arg) ((arg)*((list)._ptr + (list)._count++))

// Removes the list, not need in this implementation
#define va_end(list) ((void)0)

#define va_copy(dst, src) dst._ptr = (src)._ptr; (dst)._count = (src)._count

#endif //COMPUTER_STDARG_H
