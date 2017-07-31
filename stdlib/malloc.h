//
// Created by cout970 on 2017-07-23.
//

#ifndef DRIVER_MALLOC_H
#define DRIVER_MALLOC_H

/**
 * Original implementation: http://www.flipcode.com/archives/Simple_Malloc_Free_Functions.shtml
 */

#define USED 1

typedef struct {
    unsigned size;
} UNIT;

typedef struct {
    UNIT *free;
    UNIT *heap;
} MALLOC_HEAP;

static MALLOC_HEAP malloc_heap;

static UNIT *compact(UNIT *p, unsigned nsize) {
    unsigned bsize, psize;
    UNIT *best;

    best = p;
    bsize = 0;

    while (psize = p->size, psize) {
        if (psize & USED) {
            if (bsize != 0) {
                best->size = bsize;
                if (bsize >= nsize) {
                    return best;
                }
            }
            bsize = 0;
            best = p = (UNIT *) ((unsigned) p + (psize & ~USED));
        } else {
            bsize += psize;
            p = (UNIT *) ((unsigned) p + psize);
        }
    }

    if (bsize != 0) {
        best->size = bsize;
        if (bsize >= nsize) {
            return best;
        }
    }

    return 0;
}

void malloc_free(void *ptr) {
    if (ptr) {
        UNIT *p;

        p = (UNIT *) ((unsigned) ptr - sizeof(UNIT));
        p->size &= ~USED;
    }
}

void *malloc_alloc(unsigned size) {
    unsigned fsize;
    UNIT *p;

    if (size == 0) return 0;

    size += 3 + sizeof(UNIT);
    size >>= 2;
    size <<= 2;

    if (malloc_heap.free == 0 || size > malloc_heap.free->size) {
        malloc_heap.free = compact(malloc_heap.heap, size);
        if (malloc_heap.free == 0) return 0;
    }

    p = malloc_heap.free;
    fsize = malloc_heap.free->size;

    if (fsize >= size + sizeof(UNIT)) {
        malloc_heap.free = (UNIT *) ((unsigned) p + size);
        malloc_heap.free->size = fsize - size;
    } else {
        malloc_heap.free = 0;
        size = fsize;
    }

    p->size = size | USED;

    return (void *) ((unsigned) p + sizeof(UNIT));
}

void malloc_init(void *heap, unsigned len) {
    len += 3;
    len >>= 2;
    len <<= 2;
    malloc_heap.free = malloc_heap.heap = (UNIT *) heap;
    malloc_heap.free->size = malloc_heap.heap->size = len - sizeof(UNIT);
    *(unsigned *) ((char *) heap + len - 4) = 0;
}

void malloc_compact(void) {
    malloc_heap.free = compact(malloc_heap.heap, 0x7FFFFFFF);
}

#endif //DRIVER_MALLOC_H
