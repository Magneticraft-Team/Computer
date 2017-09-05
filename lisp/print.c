//
// Created by cout970 on 2017-08-12.
//

#include "lisp.h"

void printObjFormatted(Object *op) {
    if (op == NULL) {
        printf("NULL");
        return;
    }
    switch (op->type) {
        case INT:
            printf("%d", getInt(op));
            break;
        case CONS:
            printf("(");
            for (;;) {
                printObjFormatted(getFirst(op));
                if (isNil(getRest(op))) {
                    printf(")");
                    break;
                }
                op = getRest(op);
                if (op->type != CONS) {
                    printf(" . ");
                    printObjFormatted(op);
                    printf(")");
                    break;
                }
                printf(" ");
            }
            break;
        case SYM:
            (isNil(op)
             ? printf("nil")
             : printf("%s", getSymbolName(op)));
            break;
        case PRIMOP:
            printf("#<native function>");
            break;
        case PROC:
            printf("#<function>");
            break;
        default:
            printf("Unknown object type: %d", op->type);
            longjmp(onError, 3);
    }
}

void printObj(Object *obj) {
    if (obj->type == SYM) {
        printf("SYM(%s)", getSymbolName(obj));
    } else if (obj->type == INT) {
        printf("INT(%d)", getInt(obj));
    } else if (obj->type == CONS) {
        printf("CONS(%d)", obj->hash);
    } else if (obj->type == PROC) {
        printf("PROC(args = ");
        printObjFormatted(getProcArgs(obj));
        printf(", code = ");
        printObjFormatted(getProcCode(obj));
        printf(", env = ");
        printObjFormatted(getProcEnv(obj));
        printf(")");
    } else if (obj->type == PRIMOP) {
        printf("PRIMOD(0x%x)", (int) getPrimop(obj));
    }
}