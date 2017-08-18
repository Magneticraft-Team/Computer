/* Original implementation: https://github.com/kristianlm/small-lisp */


#include "dependencies.h"
#include "words.h"
#include "dictionary.h"

int exitCode = 0;

/*** Main Driver ***/
int main() {
    clear_screen();
    printf("FORTH 1.0\n");
    init();

    while (!exitCode){
        fun_forth(dictionary);
    }

    return exitCode;
}