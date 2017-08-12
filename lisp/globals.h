//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_GLOBALS_H
#define MAGNETICRAFTCOMPUTER_GLOBALS_H

#include "dependencies.h"
#include "object.h"

extern const char *objTypeNames[];

extern jmp_buf onError;
// current interpreter line
extern int line_num;
// total memory allocated
extern int total_malloc;
// total number of string allocated
extern int string_count;
// used to not print evaluation output
extern int write_output_flag;
// last object index, used to get the unique number of each object
extern int lastNum;

//used for garbage collection
extern Object *all_objects;

extern Object *all_symbols, *top_env;
extern Object *nil, *tee, *quote, *s_if, *s_lambda, *s_define, *s_setb, *s_begin, *s_defun;

#endif //MAGNETICRAFTCOMPUTER_GLOBALS_H
