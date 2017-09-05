//
// Created by cout970 on 2017-08-12.
//

#include "lisp.h"

const char *objTypeNames[] = {"int", "symbol", "cons/list", "function", "native function"};

jmp_buf onError;
// current interpreter line
int line_num = 0;
// total memory allocated
int total_malloc = 0;
// total number of string allocated
int string_count = 0;
// used to not print evaluation output
int write_output_flag = 0;
// last object index, used to get the unique number of each object
int lastNum = 0;

//used for garbage collection
Object *all_objects = NULL;

Object *all_symbols, *top_env;
Object *nil, *tee, *quote, *s_if, *s_lambda, *s_define, *s_setb, *s_begin, *s_defun;