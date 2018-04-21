

#include <debug.h>
#include <motherboard.h>
#include <kprint.h>
#include "../include/reader.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/gc.h"

void main() {
    monitor_clear(motherboard_get_monitor());
    kprint("Lisp 2.0\n");

#ifdef COMPUTER_ENV
    fs_init(motherboard_get_floppy_drive());
#endif

    gc_init();
    rd_init();
    lx_init();
    pr_init();
    in_init();

    kprint("%d bytes free\n", gc_free());
    kprint("Type '(help)' for available commands\n", gc_free());

    in_run();

    kprint("EOP\n");
}