//
// Created by cout970 on 2017-08-12.
//

#include "gc.h"
#include "dependencies.h"
#include "globals.h"
#include "getters.h"
#include "tokenizer.h"


/*** Garbage Collector ***/

void mark(Object *obj) {
    if (obj->marked == 1) return;

    obj->marked = 1;
    if (obj->type == CONS) {
        mark(getFirst(obj));
        mark(getRest(obj));
    } else if (obj->type == PROC) {
        mark(getProcArgs(obj));
        mark(getProcCode(obj));
        mark(getProcEnv(obj));
    }
}

void markAll() {
    mark(top_env);
    mark(all_symbols);
}


void freeObj(Object *obj) {
    int objSize = 0;

    if (obj->type == SYM) {
        objSize = sizeof(Object) + sizeof(Object *);
        freeSymbolString(getSymbolName(obj));
    } else if (obj->type == INT) {
        objSize = sizeof(Object) + sizeof(Object *);
    } else if (obj->type == CONS) {
        objSize = sizeof(Object) + 2 * sizeof(Object *);
    } else {
        printf("Error trying to free a object of type: %s\n", objTypeNames[obj->type]);
        longjmp(onError, 8);
    }
    free(obj);
    total_malloc -= objSize;
}

void freeSymbolString(const char *token) {
    total_malloc -= strlen(token);
    string_count--;
    free((void *) token);
}


void sweep() {
    Object *obj;
    Object *prev, *aux;
    int objCount = 0, removed = 0, count = 0;

    obj = all_objects;
    prev = NULL;
    while (obj != NULL) {
        objCount++;
        if (!obj->marked) {
            removed++;

            if (prev != NULL) {
                prev->next = obj->next;
            } else {
                all_objects = obj->next;
            }
            aux = obj;
            obj = (Object *) obj->next;
            freeObj(aux);
            continue;
        }
        prev = obj;
        obj = (Object *) obj->next;
    }

    for (obj = all_objects; obj != NULL; obj = (Object *) obj->next) {
        obj->marked = 0;
        count++;
    }

    printf("Before gc: %d, Removed objects: %d, Total Objects: %d\n", objCount, removed, count);
}