//
// Created by cout970 on 19/09/17.
//

#ifndef UNTITLED_INTERPRETER_H
#define UNTITLED_INTERPRETER_H

#include "reader.h"
#include "object.h"
#include "parser.h"

void in_init();

void in_run();

Object *lookupSymbol(Object *sym, Object *env);

Object *callFunction(Object *func, Object *args, Object *env);

Object *addArgsToEnv(Object *env, Object *syms, Object *vals);

Object *assoc(Object *key, Object *env);

Object *eval(Object *code, Object *env);

Object *evalList(Object *exps, Object *env);

Object *expandMacro(Object *macro, Object *env);

Object *extendEnv(Object *sym, Object *val);

Object *reverse(Object *list);

#endif //UNTITLED_INTERPRETER_H
