/* Original implementation: https://github.com/kristianlm/small-lisp */


#include "dependencies.h"
#include "asm.h"

#include "../driver/api/boot.h"

/*** Main Driver ***/
void main() {
    clear_screen();
    printf("ASM 1.0\n");
    printf("Type ':help' for command list\n");

    while (!loopTick());
}