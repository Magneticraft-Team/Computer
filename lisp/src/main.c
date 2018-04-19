

#include <debug.h>
#include <motherboard.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/interpreter.h"
#ifdef COMPUTER_ENV
#include <util/malloc.h>
extern char *__end;
#endif

void main() {
    monitor_clear(motherboard_get_monitor());
    kdebug("Lisp 2.0\n");

#ifdef COMPUTER_ENV
    initHeap(&__end + 340, motherboard_get_memory_size() - (Int)(&__end + 340) - 1024);
    fs_init(motherboard_get_floppy_drive());
#endif

    rd_init();
    lx_init();
    pr_init();
    in_init();

    in_run();

    kdebug("EOP\n");
}