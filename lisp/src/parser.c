//
// Created by cout970 on 19/09/17.
//

#include <debug.h>
#include "../include/lexer.h"
#include "../include/object.h"
#include "../include/functions.h"
#include "../include/exception.h"


void pr_init() {
    // ignore
}

Object *readObj2(Token tk);

Object *readObjList2(Token tk);


#ifndef ENV_DEBUG

long int strtol(const char *str, char **endptr, int base) {
    long int acum = 0;
    int pos = 0;
    int sign = 1;

    if (str[pos] == '-') {
        sign = -1;
        pos++;
    } else if (str[pos] == '+') {
        pos++;
    }

    while (str[pos] != '\0') {
        int val;
        if (str[pos] >= '0' && str[pos] <= '9') {
            val = str[pos] - '0';
        } else if (str[pos] >= 'a' && str[pos] <= 'z') {
            val = str[pos] - 'a' + 10;
        } else if (str[pos] >= 'A' && str[pos] <= 'Z') {
            val = str[pos] - 'A' + 10;
        } else {
            val = -1;
        }
        if (val < 0 || val >= base) {
            *endptr = (char *) &str[pos];
            return acum;
        }
        acum = acum * base + val;
        pos++;
    }
    *endptr = (char *) &str[pos];

    return acum * sign;
}

#endif

Object *readObj() {
    Token tk;
    lx_nextToken(&tk);
    return readObj2(tk);
}

Object *readObjList() {
    Token tk;
    lx_nextToken(&tk);
    return readObjList2(tk);
}

Object *readObj2(Token tk) {
    if (tk.type == TK_EOF) return NULL;
    if (tk.type == TK_NUMBER) {
        char *end;
        return createNumber((int) strtol(tk.name, &end, 10));
    }
    if (tk.type == TK_IDENTIFIER) return createSymbol(tk.name);
    if (tk.type == TK_STRING) return createString(tk.name);

    if (tk.type == TK_LEFT_PAREN) return readObjList();
    if (tk.type == TK_QUOTE) {
        Object *obj = readObj();
        if (obj == NULL) return NULL;

        return createCons(obj_quote, createCons(obj, obj_nil));
    }
    kdebug("Invalid Token: type = %d", tk.type);
    THROW(EXCEPTION_INVALID_TOKEN);
    return NULL;
}


Object *readObjList2(Token tk) {
    if (tk.type == TK_EOF) return NULL;
    if (tk.type == TK_RIGHT_PAREN) return obj_nil;

    Object *obj = readObj2(tk);
    if (obj == NULL) return NULL;

    Object *list = readObjList();
    if (list == NULL) return NULL;

    return createCons(obj, list);
}

Object *pr_parse() {
    return readObj();
}
