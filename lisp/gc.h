//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_GC_H
#define MAGNETICRAFTCOMPUTER_GC_H

#include "object.h"

void markAll();

void mark(Object *obj);

void sweep();

void freeObj(Object *obj);

void freeSymbolString(const char *token);

#endif //MAGNETICRAFTCOMPUTER_GC_H
