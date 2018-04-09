//
// Created by cout970 on 14/03/18.
//

#ifndef COMPUTER_BUF_H
#define COMPUTER_BUF_H

#include <math.h>
#include <assert.h>
#include <malloc.h>

// stretchy buffers, invented (?) by sean barrett
/*
 * Basic implementation a buffer that automatically grows, it allows direct access to the elements
 * Usage example:
 *
 * void foo(){
 *      int *myBuffer = NULL;
 *
 *      buf_push(myBuffer, 1);
 *      buf_push(myBuffer, 2);
 *      buf_push(myBuffer, 3);
 *
 *      for(int i = 0; i < buf_len(myBuffer); i++){
 *
 *          printf("%d\n", myBuffer[i]);
 *      }
 *
 *      buf_free(myBuffer);
 * }
 *
 */

typedef struct BufHdr {
    int len;
    int cap;
    char buf[0];
} BufHdr;

#ifndef offsetof
typedef unsigned int size_t;
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))

#ifndef buf__grow
void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = MAX(16, MAX(1 + 2*buf_cap(buf), new_len));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = realloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = malloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}
#endif

#endif //COMPUTER_BUF_H
