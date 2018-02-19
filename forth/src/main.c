/* Original implementation: https://github.com/kristianlm/small-lisp */


#include <motherboard.h>
#include <debug.h>
#include "../include/dependencies.h"
#include "../include/words.h"

int exitCode = 0;

void main()  {

    monitor_clear(motherboard_get_monitor());
    kdebug("FORTH 1.1\n");
    init();

    while (!exitCode){
        fun_forth();
    }
}