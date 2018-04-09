//
// Created by cout970 on 20/09/17.
//

#include <debug.h>
#include <malloc.h>
#include "../include/gc.h"

#ifndef offsetof
#include <stddef.h>
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

struct metadata {
    struct Chunk *next;
    int mark;
};

typedef struct Chunk {
    struct metadata meta;
    Object obj;
} Chunk;

int totalChunks = 0;

static Chunk *last = NULL;

char *strCopy(const char *src) {
    if (!src) return NULL;

    int len = (int) strlen(src);
    char *newStr = malloc(len + 1);
    memcpy(newStr, src, len + 1);
    return newStr;
}

Object *objAlloc() {
    Chunk *chunk = malloc(sizeof(Chunk));
    chunk->meta.next = NULL;
    chunk->meta.mark = 0;

    chunk->meta.next = last;
    last = chunk;
    totalChunks++;

    return &chunk->obj;
}

void mark(Object *obj) {

    Chunk *chunk = (Chunk *) ((char *) obj - offsetof(Chunk, obj));
    if (chunk->meta.mark) return;

    chunk->meta.mark = 1;

    if (obj->type == CONS) {
        mark(getFirst(obj));
        mark(getRest(obj));
    } else if (obj->type == FUNC) {
        mark(obj->code);
        mark(obj->args);
        mark(obj->env);
    }
}

void sweep() {
    Chunk *lastCheck = NULL;
    for (Chunk *chunk = last; chunk != NULL;) {

        if (!chunk->meta.mark) {

            if (lastCheck == NULL) {
                last = chunk->meta.next;
            } else {
                lastCheck->meta.next = chunk->meta.next;
            }
            Chunk *aux = chunk;
            chunk = chunk->meta.next;
            free(aux);
            totalChunks--;
        } else {
            lastCheck = chunk;
            chunk = chunk->meta.next;
        }
    }

    for (Chunk *chunk = last; chunk != NULL; chunk = chunk->meta.next) {
        chunk->meta.mark = 0;
    }
}

void gc(Object *env) {
    mark(env);
    sweep();
}