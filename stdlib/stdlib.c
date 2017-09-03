//
// Created by cout970 on 2016-10-23.
//

#include "dependencies.h"
#include "api/stdlib.h"
#include "api/stdio.h"

////////////// STRING CONVERSIONS //////////////

//BUG this doesn't work with LONG_MAX
int atoi(const char *str) {
    unsigned long int acum = 0;
    int sign = 1;
    int pos = 0;
    if (str[0] == '-') {
        pos = 1;
        sign = -1;
    } else if (str[0] == '+') {
        pos = 1;
        sign = 1;
    }

    while (str[pos] != '\0') {
        int val = str[pos] - '0';
        if (val < 0 || val > 9) {
            return 0;
        }
        acum = acum * 10 + val;
        pos++;
    }

    return ((int) acum) * sign;
}

long int atol(const char *str) {
    return atoi(str);
}

long int strtol(const char *str, char **endptr, int base) {
    long int acum = 0;
    int pos = 0;
    int sign = 1;

    if (str[pos] == '-') {
        sign = -1;
        pos++;
    } else if (str[pos] == '+') {
        pos++;
    }

    while (str[pos] != '\0') {
        int val;
        if (str[pos] >= '0' && str[pos] <= '9') {
            val = str[pos] - '0';
        } else if (str[pos] >= 'a' && str[pos] <= 'z') {
            val = str[pos] - 'a' + 10;
        } else if (str[pos] >= 'A' && str[pos] <= 'Z') {
            val = str[pos] - 'A' + 10;
        } else {
            val = -1;
        }
        if (val < 0 || val >= base) {
            *endptr = (char *) &str[pos];
            return acum;
        }
        acum = acum * base + val;
        pos++;
    }
    *endptr = (char *) &str[pos];

    return acum * sign;
}

unsigned long int strtoul(const char *str, char **endptr, int base) {
    unsigned int acum = 0;
    int pos = 0;

    while (str[pos] != '\0') {
        int val;
        if (str[pos] >= '0' && str[pos] <= '9') {
            val = str[pos] - '0';
        } else if (str[pos] >= 'a' && str[pos] <= 'z') {
            val = str[pos] - 'a' + 10;
        } else if (str[pos] >= 'A' && str[pos] <= 'Z') {
            val = str[pos] - 'A' + 10;
        } else {
            val = -1;
        }
        if (val < 0 || val >= base) {
            *endptr = (char *) &str[pos];
            return acum;
        }
        acum = acum * base + val;
        pos++;
    }

    return acum;
}

////////////// MEMORY MANAGEMENT //////////////

#define USED 1

typedef struct {
    unsigned size;
} UNIT;

typedef struct {
    UNIT *free;
    UNIT *heap;
} MALLOC_HEAP;

static MALLOC_HEAP malloc_heap;
static int malloc_init_flag = 0;

static void malloc_init(void *heap, unsigned len) {
    len += 3;
    len >>= 2;
    len <<= 2;
    malloc_heap.free = malloc_heap.heap = (UNIT *) heap;
    malloc_heap.free->size = malloc_heap.heap->size = len - sizeof(UNIT);
    *(unsigned *) ((char *) heap + len - 4) = 0;
}

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

void malloc_compact(void) {
    malloc_heap.free = compact(malloc_heap.heap, 0x7FFFFFFF);
}

void free(void *ptr) {
    if (ptr) {
        UNIT *p;

        p = (UNIT *) ((unsigned) ptr - sizeof(UNIT));
        p->size &= ~USED;
    }
}

// program end address, the end of the bss program section
extern char *__end;

void *malloc(unsigned size) {
    unsigned fsize;
    UNIT *p;

    if (!malloc_init_flag) {
        fsize = (unsigned int) &__end;
        // offset to avoid data corruption
        fsize += 16;
        // aligning to word boundaries
        fsize += 3;
        fsize &= ~3;
        //init
        malloc_init((void *) fsize, motherboard_get_memory_size() - fsize);
        malloc_init_flag = 1;
        fsize = 0;
    }
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

void *calloc(size_t nitems, size_t size) {
    size_t bytes = nitems * size;
    if (bytes == 0) return NULL;
    int *ptr = (int *) malloc(bytes);
    int size_ = (bytes + 3) / 4;
    for (int i = 0; i < size_; i++) {
        ptr[i] = 0;
    }
    return ptr;
}

void *realloc(void *ptr IGNORED, size_t size IGNORED) {
    //TODO
    //this needs a allocation table
    return NULL;
}

////////////// EXIT //////////////

void abort() {
    exit(-1);
}

int atexit(void (*func)() IGNORED) {
    // TODO run func at program exit
    return 0;
}

void exit(int status IGNORED) {
    motherboard_signal(MOTHERBOARD_SIGNAL_HALT);
}

////////////// ENVIROMENT //////////////

char *getenv(const char *str IGNORED) {
    return (char *) NULL;
}

int system(const char *str IGNORED) {
    return 0;
}

////////////// ARRAY UTILITIES //////////////

//void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

//void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

////////////// MATH //////////////

int abs(int x) {
    return (x < 0) ? -x : x;
}

div_t div(int numer, int demom) {
    div_t d;
    d.quot = numer / demom;
    d.rem = numer % demom;
    return d;
}

long int labs(long int x) {
    return abs(x);
}

ldiv_t ldiv(long int numer, long int demom) {
    ldiv_t d;
    d.quot = numer / demom;
    d.rem = numer % demom;
    return d;
}
