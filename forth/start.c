/* Original implementation: https://github.com/kristianlm/small-lisp */


#include "dependencies.h"
#include "words.h"
#include "dictionary.h"

int exitCode = 0;

// needed to start execution at main
#include "../driver/api/boot.h"

/*** Main Driver ***/
void main()  {

    clear_screen();
    printf("FORTH 1.0\n");
    init();

    while (!exitCode){
        fun_forth();
    }
}