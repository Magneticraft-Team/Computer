/* Original implementation: https://github.com/kristianlm/small-lisp
   A minimal Lisp interpreter
   Copyright 2004 Andru Luvisi

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License , or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "../driver/api/motherboard.h"
#include "../driver/api/monitor.h"
#include "../stdlib/std/stdio.h"
#include "../stdlib/std/stdlib.h"
#include "../stdlib/std/stdarg.h"
#include "../stdlib/std/string.h"
#include "../stdlib/std/ctype.h"
#include "../stdlib/std/assert.h"

int line_num = 0;
int total_malloc = 0;
int clear_flag = 0;

/*** List Structured Memory ***/
enum ObjectType {
    INT,    // number
    SYM,    // symbol
    CONS,   // pair
    PROC,   // lazy function evaluation
    PRIMOP  // c function
};

typedef struct obj {
    enum ObjectType type;
    int line_num;
    struct obj *data[1];
} Object;

// Define Primop as Object* -> Object*
typedef Object *(*Primop)(Object *);

Object *all_symbols, *top_env, *nil, *tee, *quote, *s_if, *s_lambda, *s_define, *s_setb, *s_begin, *s_defun;

// base
Object *newObject(enum ObjectType type, int count, ...) {
    Object *ret;
    int i;
    va_list ap;
    va_start(ap, count);

    int object_size = sizeof(Object) + (count - 1) * sizeof(Object *);
    total_malloc += object_size;

    ret = (Object *) malloc((size_t) object_size);
    ret->type = type;
    ret->line_num = line_num;

    for (i = 0; i < count; i++) ret->data[i] = va_arg(ap, Object *);

    va_end(ap);
    return ret;
}

inline Object *getCar(Object *X) {
    if (X == 0) {
        fprintf(stderr, "warning: car argument null on line %d\n", line_num);
        return nil;
    }
    if (X == nil)
        return nil;
    if (X->type != CONS) {
        fprintf(stderr, "warning: car argument not a list (%d) on line %d\n", (int) X->data[0], X->line_num);
        return nil;
    }
    return X->data[0];
}

inline Object *getCdr(Object *X) {
    if (X == nil)
        return nil;

    if (X->type != CONS) {
        fprintf(stderr, "warning: cdr argument not a list on line %d\n", X->line_num);
        return nil;
    }
    if (X->data[1] == 0) {
        fprintf(stderr, "error: cdr list element is zero-pointer at %d\n", X->line_num);
        return nil;
    }
    return X->data[1];
}

inline Object *getFirst(Object *obj) {
    return getCar(obj);
}

inline Object *getRest(Object *obj) {
    return getCdr(obj);
}

inline void setCdr(Object *a, Object *cdr) {
    a->data[1] = cdr;
}

//inline void setCar(Object *a, Object *car) { a->data[0] = car; }

// constructors

inline Object *createCons(Object *car, Object *cdr) {
    return newObject(CONS, 2, car, cdr);
}

inline Object *createProc(Object *a, Object *b, Object *c) {
    return newObject(PROC, 3, a, b, c);
}

inline Object *createPrimop(Primop func) {
    return newObject(PRIMOP, 1, (Object *) func);
}

inline Object *createSymbol(const char *symbol) {
    return newObject(SYM, 1, (Object *) symbol);
}

inline Object *createInt(int value) {
    return newObject(INT, 1, value);
}

// getters

inline Primop getPrimop(Object *obj) {
    return (Primop) obj->data[0];
}

inline int getInt(Object *obj) {
    return obj->type == INT ? (int) obj->data[0] : 0;
}

inline Object *getProcArgs(Object *obj) {
    return obj->data[0];
}

inline Object *getProcCode(Object *obj) {
    return obj->data[1];
}

inline Object *getProcEnv(Object *obj) {
    return obj->data[2];
}

inline char *getSymbolName(Object *obj) {
    return (char *) (obj->data[0]);
}

int isNil(Object *obj) {
    return obj == nil;
}

Object *getElem(Object *list, int index) {
    Object *aux = list;
    int i;
    for (i = 0; i < index; i++) {
        aux = getRest(aux);
    }
    return getFirst(aux);
}

// parser

inline int isEq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

Object *findSymbol(const char *name) {
    Object *symbolList;

    for (symbolList = all_symbols; !isNil(symbolList); symbolList = getRest(symbolList)) {
        if (isEq(name, getSymbolName(getFirst(symbolList)))) {
            return symbolList;
        }
    }
    return nil;
}

Object *prim_print(Object *a);

Object *intern(const char *name) {
    Object *op = findSymbol(name);

    if (!isNil(op)) {
        return getFirst(op);
    }
    op = createSymbol(name);
    all_symbols = createCons(op, all_symbols);
    return op;
}

/*** Environment ***/

Object *multiple_extend(Object *env, Object *syms, Object *vals) {
    return isNil(syms)
           ? env
           : multiple_extend(createCons(createCons(getFirst(syms), getFirst(vals)), env), getRest(syms), getRest(vals));
}

Object *extend_top(Object *sym, Object *val) {
    Object *pair = createCons(sym, val);
    //insert node
    setCdr(top_env, createCons(pair, getRest(top_env)));
    return val;
}

Object *assoc(Object *key, Object *list) {
    if (isNil(list)) return nil;

    if (getFirst(getFirst(list)) == key) return getFirst(list);
    return assoc(key, getRest(list));
}

/*** Input/Output ***/

#define INPUT_BUFFER_SIZE 80
char inputBuffer[INPUT_BUFFER_SIZE];
int readBuffPtr = 0;

char tokenBuffer[100];
int tokenBufferPtr;

const char *tokenLook;
int mustReadToken = 1;

void readLineBuffered() {
    readBuffPtr = 0;
    line_num++;
    printf("%d >", line_num);
    fgets(inputBuffer, INPUT_BUFFER_SIZE - 1, stdin);
    int len = strlen(inputBuffer);
    inputBuffer[len] = ' ';
    inputBuffer[len + 1] = '\0';
    putchar('\n');
}

const char *saveString(const char *str) {
    total_malloc += strlen(str);
    return strdup(str);
}

void readToken() {
    char comment = 0;
    tokenBufferPtr = 0;
    char currentChar;
    do {
        currentChar = inputBuffer[readBuffPtr++];
        if (currentChar == '\0' || readBuffPtr >= INPUT_BUFFER_SIZE) {
            comment = 0;
            readLineBuffered();
        }
        if (currentChar == ';') comment = 1;
    } while (isspace(currentChar) || comment || currentChar == '\0');

    tokenBuffer[tokenBufferPtr++] = currentChar;
    if (strchr("()\'", currentChar)) {
        tokenBuffer[tokenBufferPtr++] = '\0';
        tokenLook = saveString(tokenBuffer);
        return;
    }
    for (;;) {
        currentChar = inputBuffer[readBuffPtr++];
        if (currentChar == '\0') {
            readLineBuffered();
        }
        if (strchr("()\'", currentChar) || isspace(currentChar)) {
            readBuffPtr--;
            tokenBuffer[tokenBufferPtr++] = '\0';
            tokenLook = saveString(tokenBuffer);
            return;
        }
        tokenBuffer[tokenBufferPtr++] = currentChar;
    }
}

const char *getToken() {
    if (mustReadToken) {
        readToken();
    }
    mustReadToken = 1;
    return tokenLook;
}

Object *readlist();

Object *readObj() {

    const char *token = getToken();

    if (isEq(token, "(")) {
        return readlist();
    }
    if (isEq(token, "\'")) {
        return createCons(quote, createCons(readObj(), nil));
    }

    if (token[strspn(token, "0123456789")] == '\0' || (token[0] == '-' && strlen(token) > 1)) {
        return createInt(atoi(token));
    }
    return intern(token);
}

Object *readlist() {
    const char *token = getToken();
    Object *tmp;
    if (isEq(token, ")")) return nil;
    if (isEq(token, ".")) {
        tmp = readObj();
//        if (!isEq(getToken(), ")") != 0) exit(1);
        return tmp;
    }
    mustReadToken = 0;
    tmp = readObj(); /* Must force evaluation order */
    return createCons(tmp, readlist());
}

void writeObj(FILE *ofp, Object *op) {
    switch (op->type) {
        case INT:
            fprintf(ofp, "%d", getInt(op));
            break;
        case CONS:
            fprintf(ofp, "(");
            for (;;) {
                writeObj(ofp, getFirst(op));
                if (isNil(getRest(op))) {
                    fprintf(ofp, ")");
                    break;
                }
                op = getRest(op);
                if (op->type != CONS) {
                    fprintf(ofp, " . ");
                    writeObj(ofp, op);
                    fprintf(ofp, ")");
                    break;
                }
                fprintf(ofp, " ");
            }
            break;
        case SYM:
            (isNil(op)
             ? fprintf(ofp, "()")
             : fprintf(ofp, "%s", getSymbolName(op)));
            break;
        case PRIMOP:
            fprintf(ofp, "#<PRIMOP>");
            break;
        case PROC:
            fprintf(ofp, "#<PROC>");
            break;
        default:
            exit(1);
    }
}

/*** Evaluator (Eval/no Apply) ***/
Object *evlis(Object *exps, Object *env);

Object *eval(Object *exp, Object *env) {
    Object *tmp, *proc, *vals, *ifExp;

    eval_start:

    if (exp == nil) return nil;

    switch (exp->type) {
        case INT:
            return exp;
        case PRIMOP:
            return exp;
        case PROC:
            return exp;
        case SYM:
            //look up value
            tmp = assoc(exp, env);

            if (tmp == nil) {
                fprintf(stderr, "Unbound symbol ");
                writeObj(stderr, exp);
                fprintf(stderr, "\n");
                return nil;
            }
            return getRest(tmp);
        case CONS:
            if (getFirst(exp) == s_if) { // IF
                ifExp = getFirst(getRest(exp));
                if (eval(ifExp, env) != nil)
                    return eval(getElem(exp, 2), env);
                else
                    return eval(getElem(exp, 3), env);
            }

            if (getFirst(exp) == s_lambda) // LAMBDA
                return createProc(getElem(exp, 1), getRest(getRest(exp)), env);

            if (getFirst(exp) == quote) // QUOTE '
                return getFirst(getRest(exp));

            if (getFirst(exp) == s_define) // DEFINE
                return (extend_top(getElem(exp, 1), eval(getElem(exp, 2), env)));

            if (getFirst(exp) == s_setb) { // SET!
                Object *pair = assoc(getElem(exp, 1), env);
                Object *newval = eval(getElem(exp, 2), env);
                setCdr(pair, newval);
                return newval;
            }

            if (getFirst(exp) == s_begin) { // BEGIN
                exp = getRest(exp);
                if (exp == nil) return nil;
                for (;;) {
                    if (getRest(exp) == nil) {
                        exp = getFirst(exp);
                        goto eval_start;
                    }
                    eval(getFirst(exp), env);
                    exp = getRest(exp);
                }
            }

            if(getFirst(exp) == s_defun){ // DEFUN

            }

            proc = eval(getFirst(exp), env);
            vals = evlis(getRest(exp), env);

            if (proc->type == PRIMOP) // C FUNCTION
                return (*getPrimop(proc))(vals);

            if (proc->type == PROC) { // COMPOSITE FUNCTION
                env = multiple_extend(getProcEnv(proc), getProcArgs(proc), vals);
                exp = createCons(s_begin, getProcCode(proc));
                goto eval_start;
            }
            printf("Bad PROC type\n");
            return nil;
    }
    /* Not reached */
    return exp;
}

Object *evlis(Object *exps, Object *env) {
    if (exps == nil) return nil;
    return createCons(eval(getFirst(exps), env),
                      evlis(getRest(exps), env));
}

/*** Primitives ***/
Object *prim_sum(Object *args) {
    int sum;
    for (sum = 0; !isNil(args); sum += getInt(getFirst(args)), args = getRest(args));
    return createInt(sum);
}

Object *prim_sub(Object *args) {
    int sum;
    for (sum = getInt(getFirst(args)), args = getRest(args);
         !isNil(args);
         sum -= getInt(getFirst(args)), args = getRest(args));
    return createInt(sum);
}

Object *prim_prod(Object *args) {
    int prod;
    for (prod = 1; !isNil(args); prod *= getInt(getFirst(args)), args = getRest(args));
    return createInt(prod);
}

Object *prim_divide(Object *args) {
    int prod = getInt(getFirst(args));
    args = getRest(args);
    while (!isNil(args)) {
        prod /= getInt(getFirst(args));
        args = getRest(args);
    }
    return createInt(prod);
}

Object *prim_gt(Object *args) {
    return getInt(getFirst(args)) > getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_lt(Object *args) {
    return getInt(getFirst(args)) < getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_ge(Object *args) {
    return getInt(getFirst(args)) >= getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_le(Object *args) {
    return getInt(getFirst(args)) <= getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_numeq(Object *args) {
    return getInt(getFirst(args)) == getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_cons(Object *args) { return createCons(getFirst(args), getFirst(getRest(args))); }

Object *prim_car(Object *args) { return getFirst(getFirst(args)); }

Object *prim_cdr(Object *args) { return getRest(getFirst(args)); }


/*** Helpers *****/

Object *prim_print(Object *args) {
    if (isNil(args)) {
        printf("nil\n");
        return nil;
    }
    while (!isNil(args)) {
        writeObj(stdout, getFirst(args));
        args = getRest(args);
        printf(" ");
    }
    printf("\n");
    return nil;
}

Object *prim_clear(Object *args IGNORED) {
    clear_screen();
    monitor_set_cursor_pos_y(motherboard_get_monitor(), 0);
    clear_flag = 1;
    return nil;
}

Object *prim_free(Object *args) {
    if (!isNil(args)) { printf(""); }
    int used = (int) malloc(4);
    free((void *) used);

    int free = motherboard_get_memory_size() - used;
    printf("%d bytes left hanging, %d bytes free\n", total_malloc, free);
    return nil;
}

/*** Initialization ***/
void init_sl3() {
    nil = createSymbol("nil");
    all_symbols = createCons(nil, nil);
    top_env = createCons(createCons(nil, nil), nil);
    tee = intern("t");
    extend_top(tee, tee);
    quote = intern("quote");
    s_if = intern("if");
    s_lambda = intern("lambda");
    s_define = intern("define");
    s_defun = intern("defun");
    s_setb = intern("set!");
    s_begin = intern("begin");
    extend_top(intern("+"), createPrimop(prim_sum));
    extend_top(intern("-"), createPrimop(prim_sub));
    extend_top(intern("*"), createPrimop(prim_prod));
    extend_top(intern("/"), createPrimop(prim_divide));
    extend_top(intern("="), createPrimop(prim_numeq));

    extend_top(intern(">"), createPrimop(prim_gt));
    extend_top(intern(">="), createPrimop(prim_ge));

    extend_top(intern("<"), createPrimop(prim_lt));
    extend_top(intern("<="), createPrimop(prim_le));

    extend_top(intern("cons"), createPrimop(prim_cons));
    extend_top(intern("car"), createPrimop(prim_car));
    extend_top(intern("cdr"), createPrimop(prim_cdr));

    extend_top(intern("print"), createPrimop(prim_print));

    //alias
    extend_top(intern("cond"), s_if);
    extend_top(intern("true"), tee);
    extend_top(intern("false"), nil);
    extend_top(intern("null"), nil);

    extend_top(intern("free"), createPrimop(prim_free));
    extend_top(intern("clear"), createPrimop(prim_clear));
}

/*** Main Driver ***/
int main_sl3() {
    clear_screen();
    init_sl3();
    printf("Lips Interpreter 1.0\n");
    inputBuffer[0] = '\0';
    Object *input, *output;
    for (;;) {
        //read input
        input = readObj();
        //eval
        output = eval(input, top_env);
        //print output
        if (!clear_flag) {
            writeObj(stdout, output);
            printf("\n");
        }
        clear_flag = 0;
        if (output == NULL) break;
    }
    printf("Exit");
    return 0;
}