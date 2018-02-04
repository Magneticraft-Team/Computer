//
// Created by cout970 on 27/10/16.
//

#ifndef COMPUTER_ERRNO_H
#define COMPUTER_ERRNO_H


/**
 * This is the macro set by system calls and some library
 * functions in the event of an error to indicate what went wrong.
 */
extern int errno;
/**
 * Error codes
 */
#define E_NO_ERROR 0
#define E_UNKNOWN_ERROR 1


#endif //COMPUTER_ERRNO_H
