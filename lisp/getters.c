//
// Created by cout970 on 2017-08-12.
//

#include "getters.h"
#include "dependencies.h"
#include "globals.h"

/*** Getters ***/

Object *getCar(Object *obj) {
    if (obj == 0) {
        printf("Internal Error: car argument null on line %d\n", line_num);
        longjmp(onError, 6);
        return nil;
    }
    if (obj == nil)
        return nil;

    if (obj->type != CONS) {
        printf("Error: car argument not a list (%s) on line %d\n", objTypeNames[obj->type], obj->line_num);
        longjmp(onError, 6);
        return nil;
    }
    if (obj->data[0] == 0) {
        printf("Error: car list element is zero-pointer at %d\n", obj->line_num);
        longjmp(onError, 6);
        return nil;
    }
    return obj->data[0];
}

Object *getCdr(Object *obj) {
    if (obj == 0) {
        printf("Internal Error: cdr argument null on line %d\n", line_num);
        longjmp(onError, 5);
        return nil;
    }

    if (obj == nil)
        return nil;

    if (obj->type != CONS) {
        printf("Error: cdr argument not a list (%s) on line %d\n", objTypeNames[obj->type], obj->line_num);
        longjmp(onError, 5);
        return nil;
    }
    if (obj->data[1] == 0) {
        printf("Error: cdr list element is zero-pointer at %d\n", obj->line_num);
        longjmp(onError, 5);
        return nil;
    }
    return obj->data[1];
}

Object *getFirst(Object *obj) {
    return getCar(obj);
}

Object *getRest(Object *obj) {
    return getCdr(obj);
}

void setCdr(Object *a, Object *cdr) {
    a->data[1] = cdr;
}

Primop getPrimop(Object *obj) {
    return (Primop) obj->data[0];
}

int getInt(Object *obj) {
    return obj->type == INT ? (int) obj->data[0] : 0;
}

Object *getProcArgs(Object *obj) {
    return obj->data[0];
}

Object *getProcCode(Object *obj) {
    return obj->data[1];
}

Object *getProcEnv(Object *obj) {
    return obj->data[2];
}

char *getSymbolName(Object *obj) {
    return (char *) (obj->data[0]);
}

int isNil(Object *obj) {
    return obj == nil;
}

Object *getElem(Object *list, int index) {
    Object *aux = list;
    int i;
    for (i = 0; i < index; i++) {
        aux = getRest(aux);
    }
    return getFirst(aux);
}