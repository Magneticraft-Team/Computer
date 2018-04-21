//
// Created by cout970 on 20/09/17.
//

#ifndef UNTITLED_GARBAGECOLLECTOR_H
#define UNTITLED_GARBAGECOLLECTOR_H

#include "object.h"

void gc_init();

int gc_free();

Object *objAlloc();

char *strCopy(const char *src);

void gc(Object *env);

#endif //UNTITLED_GARBAGECOLLECTOR_H
