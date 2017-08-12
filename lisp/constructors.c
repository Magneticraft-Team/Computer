//
// Created by cout970 on 2017-08-12.
//

#include "dependencies.h"
#include "constructors.h"
#include "globals.h"

Object *newObject(enum ObjectType type, int count, ...) {
    Object *ret;
    int i;
    va_list ap;
    va_start(ap, count);

    int object_size = sizeof(Object) + (count - 1) * sizeof(Object *);
    total_malloc += object_size;

    ret = (Object *) malloc((size_t) object_size);
    if (ret == 0) {
        printf("Out of memory!\n");
        longjmp(onError, 7);
        return NULL;
    }
    ret->type = type;
    ret->line_num = line_num;
    ret->hash = lastNum++;
    ret->marked = 0;
    ret->next = all_objects;
    all_objects = ret;

    for (i = 0; i < count; i++) ret->data[i] = va_arg(ap, Object *);

//    printf("[NewObj: %d, size: %d, ptr: %d, type = %s]\n", ret->hash, object_size, (int) ret, objTypeNames[type]);
    va_end(ap);
    return ret;
}

/*** Constructors ***/

Object *createCons(Object *car, Object *cdr) {
    return newObject(CONS, 2, car, cdr);
}

Object *createProc(Object *args, Object *code, Object *env) {
    return newObject(PROC, 3, args, code, env);
}

Object *createPrimop(Primop func) {
    return newObject(PRIMOP, 1, (Object *) func);
}

Object *createSymbol(const char *symbol) {
    return newObject(SYM, 1, (Object *) symbol);
}

Object *createInt(int value) {
    return newObject(INT, 1, value);
}
