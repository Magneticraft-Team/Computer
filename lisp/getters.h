//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_GETTERS_H
#define MAGNETICRAFTCOMPUTER_GETTERS_H

#include "object.h"


/*** Getters ***/

Object *getCar(Object *obj);

Object *getCdr(Object *obj);

Object *getFirst(Object *obj);

Object *getRest(Object *obj);

void setCdr(Object *a, Object *cdr);

Primop getPrimop(Object *obj);

int getInt(Object *obj);

Object *getProcArgs(Object *obj);

Object *getProcCode(Object *obj);

Object *getProcEnv(Object *obj);

char *getSymbolName(Object *obj);

int isNil(Object *obj);

Object *getElem(Object *list, int index);

#endif //MAGNETICRAFTCOMPUTER_GETTERS_H
