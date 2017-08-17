/* Original implementation: https://github.com/kristianlm/small-lisp */


#include "dependencies.h"
#include "asm.h"

/*** Main Driver ***/
int main() {
    clear_screen();
    printf("ASM 1.0\n");

    while (!loopTick());

    printCode();

    return 0;
}