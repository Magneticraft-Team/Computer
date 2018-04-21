//
// Created by cout970 on 26/02/18.
//

#ifndef MAGNETICRAFTCOMPUTER_FUNCTIONS_H
#define MAGNETICRAFTCOMPUTER_FUNCTIONS_H

#include "object.h"

extern Object* obj_nil;
extern Object* obj_env;
extern Object* obj_t;
extern Object* obj_quote;
extern Object* obj_progn;
extern Object* obj_quasiquote;
extern Object* obj_unquote;
extern Object* obj_ans;

void fn_init();

#endif //MAGNETICRAFTCOMPUTER_FUNCTIONS_H
