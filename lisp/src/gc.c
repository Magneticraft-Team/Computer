//
// Created by cout970 on 20/09/17.
//

#include <debug.h>
#include <malloc.h>
#include <kprint.h>
#include <string.h>
#include <motherboard.h>
#include "../include/gc.h"

#ifndef offsetof
#include <stddef.h>
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#define WORD_ALIGN(x) ((((x) + 3) >> 2) << 2)

#ifdef COMPUTER_ENV
#include <util/malloc.h>

extern char *__end;
int heapStart = (Int) (&__end + 340);
int stackSize = 1024 * 4;
int heapSize;
#endif

typedef struct Chunk {
    struct Chunk *next;
    int mark;
    Object obj;
} Chunk;

int totalChunks = 0;
int totalStringMem = 0;

static Chunk *last = NULL;


void gc_init() {
#ifdef COMPUTER_ENV
    heapSize = motherboard_get_memory_size() - heapStart - stackSize;
    initHeap(&__end + 340, (UInt) heapSize);
#endif
}

int gc_free() {
#ifdef COMPUTER_ENV
    int usedHeap = totalChunks * sizeof(Chunk) + totalStringMem;

    return heapSize - usedHeap;
#else
    return motherboard_get_memory_size();
#endif
}

char *strCopy(const char *src) {
    if (!src) return NULL;

    size_t size = (size_t) strlen(src) + 1;

    char *newStr = malloc(size);
    memcpy(newStr, src, size);
    totalStringMem += WORD_ALIGN(size) + 4;

    return newStr;
}

static void freeChunk(Chunk *chunk) {

    totalChunks--;

    if (chunk->obj.type == SYMBOL || chunk->obj.type == KEYWORD || chunk->obj.type == STRING) {
        totalStringMem -= WORD_ALIGN(strlen(chunk->obj.name) + 1) + 4;
        free(chunk->obj.name);
    }

    free(chunk);
}


Object *objAlloc() {
    Chunk *chunk = malloc(sizeof(Chunk));
    chunk->next = NULL;
    chunk->mark = 0;

    chunk->next = last;
    last = chunk;
    totalChunks++;

    return &chunk->obj;
}

void mark(Object *obj) {

    Chunk *chunk = (Chunk *) ((char *) obj - offsetof(Chunk, obj));
    if (chunk->mark) return;

    chunk->mark = 1;
    if (obj->type == SYMBOL || obj->type == KEYWORD || obj->type == STRING ||
        obj->type == NUMBER || obj->type == NATIVE_FUN) {
        // Simple types without children

    } else if (obj->type == CONS) {
        mark(getFirst(obj));
        mark(getRest(obj));

    } else if (obj->type == FUNC) {
        mark(obj->code);
        mark(obj->args);
        mark(obj->env);

    } else if (obj->type == MACRO) {
        mark(obj->macro_code);
        mark(obj->macro_args);
        mark(obj->macro_env);

    } else {
        kprint("Error unknown object type: (%d)\n", obj->type);
    }
}

void sweep() {
    Chunk *lastCheck = NULL;
    for (Chunk *chunk = last; chunk != NULL;) {

        if (!chunk->mark) {

            if (lastCheck == NULL) {
                last = chunk->next;
            } else {
                lastCheck->next = chunk->next;
            }
            Chunk *aux = chunk;
            chunk = chunk->next;
            freeChunk(aux);
        } else {
            lastCheck = chunk;
            chunk = chunk->next;
        }
    }

    for (Chunk *chunk = last; chunk != NULL; chunk = chunk->next) {
        chunk->mark = 0;
    }
}

void gc(Object *env) {
    mark(env);
    sweep();
}