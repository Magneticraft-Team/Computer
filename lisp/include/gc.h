//
// Created by cout970 on 20/09/17.
//

#ifndef UNTITLED_GARBAGECOLLECTOR_H
#define UNTITLED_GARBAGECOLLECTOR_H

#include "object.h"

Object *objAlloc();

char *strCopy(const char *src);

void gc(Object *env);

#endif //UNTITLED_GARBAGECOLLECTOR_H
