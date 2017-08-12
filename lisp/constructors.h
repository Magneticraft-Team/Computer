//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_CONSTRUCTORS_H
#define MAGNETICRAFTCOMPUTER_CONSTRUCTORS_H

#include "object.h"

Object *createCons(Object *car, Object *cdr);

Object *createProc(Object *args, Object *code, Object *env);

Object *createPrimop(Primop func);

Object *createSymbol(const char *symbol);

Object *createInt(int value);

#endif //MAGNETICRAFTCOMPUTER_CONSTRUCTORS_H
