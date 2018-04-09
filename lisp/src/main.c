

#include <debug.h>
#include <motherboard.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/interpreter.h"
#ifdef COMPUTER_ENV
#include <util/malloc.h>
// Memory sanbox
char heap[1024 * 32];
#endif

void main() {
    monitor_clear(motherboard_get_monitor());
    kdebug("Lisp 2.0\n");

#ifdef COMPUTER_ENV
    initHeap(heap, sizeof(heap));
#endif

    rd_init();
    lx_init();
    pr_init();
    in_init();

    in_run();

    kdebug("EOP\n");
}