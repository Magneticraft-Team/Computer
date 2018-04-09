//
// Created by cout970 on 26/02/18.
//

#include <types.h>
#include <debug.h>
#include <monitor.h>
#include <motherboard.h>
#include "../include/functions.h"
#include "../include/interpreter.h"
#include "../include/exception.h"

Object *obj_nil = NULL;
Object *obj_env = NULL;
Object *obj_t = NULL;
Object *obj_quote = NULL;
Object *obj_progn = NULL;

static Object *registerSymbol(const char *name) {
    Object *sym = createSymbol(name);
    return extendEnv(sym, sym);
}

static Object *registerFunc(const char *name, NativeFunc func) {
    Object *sym = createSymbol(name);
    extendEnv(sym, createNativeFunc(func));
    return sym;
}

Object *f_if(Object *lst, Object *env) {
    //(if (exp) ifTrue ifFalse)

    Object *ifExp = getFirst(lst);

    if (eval(ifExp, env) != obj_nil) {
        return eval(getElem(lst, 2), env);
    } else {
        return eval(getElem(lst, 3), env);
    }
}

Object *f_lambda(Object *lst, Object *env) {
    // (lambda (args...) code)
    return createFun(getFirst(lst), getRest(lst), env);
}

Object *f_quote(Object *lst, Object *env IGNORED) {
    // '(listElements...)
    return getFirst(lst);
}

Object *f_quasiquote(Object *lst, Object *env IGNORED) {
    // `(listElements...)
    // TODO
    return getFirst(lst);
}

Object *f_debug(Object *lst, Object *env IGNORED) {
    // (debug function)
    Object *obj = eval(getFirst(lst), env);

    if (obj->type == FUNC) {
        return createCons(obj->args, obj->code);
    } else if (obj->type == MACRO) {
        return createCons(obj->vars, obj->template);
    }

    return obj_nil;
}

Object *f_define(Object *lst, Object *env) {
    // (define symbol value)
    return extendEnv(getElem(lst, 0), eval(getElem(lst, 1), env));
}

Object *f_set(Object *lst, Object *env) {
    // (set! symbol newValue)
    Object *sym = getElem(lst, 0);

    if (sym->type != SYMBOL) {
        kdebug("Not a symbol: ");
        printObj(sym);
        kdebug("\n");
        THROW(EXCEPTION_NOT_A_SYMBOL);
        return obj_nil;
    }

    Object *pair = assoc(sym, env);
    Object *newVal = eval(getElem(lst, 1), env);

    if (pair == obj_nil) {
        if (newVal != obj_nil) {
            extendEnv(sym, newVal);
        }
        return newVal;

    } else {
        pair->cdr = newVal;
        return newVal;

    }
}

Object *f_progn(Object *lst, Object *env) {
    // (progn
    //          line1
    //          line2
    //          ...)
    Object *aux = obj_nil;

    while (lst != obj_nil) {
        aux = eval(getFirst(lst), env);
        lst = getRest(lst);
    }

    return aux;
}

Object *f_defun(Object *lst, Object *env) {
    // (defun symbol (args...) code)
    Object *fn = createFun(getElem(lst, 1), getRest(getRest(lst)), env);
    Object *name = getElem(lst, 0);

    if (name->type != SYMBOL) {
        kdebug("Error not a symbol: ");
        printObj(name);
        kdebug("\n");
        THROW(EXCEPTION_NOT_A_SYMBOL);
        return obj_nil;
    }

    extendEnv(name, fn);
    return fn;
}

Object *f_defmacro(Object *lst, Object *env) {
    // (defmacro symbol (args...) template)
    // http://clhs.lisp.se/Body/02_df.htm
    Object *fn = createMacro(getElem(lst, 1), getRest(getRest(lst)));
    Object *name = getElem(lst, 0);

    if (name->type != SYMBOL) {
        kdebug("Error not a symbol: ");
        printObj(name);
        kdebug("\n");
        THROW(EXCEPTION_NOT_A_SYMBOL);
        return obj_nil;
    }

    extendEnv(name, fn);
    return fn;
}

Object *f_car(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getFirst(getElem(args, 0));
}

Object *f_cdr(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getRest(getElem(args, 0));
}

Object *f_cadr(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getFirst(getRest(getElem(args, 0)));
}

Object *f_atom(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0)->type != CONS) ? obj_t : obj_nil;
}

Object *f_nth(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *index = getElem(args, 0);
    Object *list = getElem(args, 1);
    return getElem(list, getNumber(index));
}

Object *f_cons(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *first = getElem(args, 0);
    Object *second = getElem(args, 1);
    return createCons(first, second);
}

Object *f_length(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *list = getFirst(args);
    int i;
    for (i = 0; list != obj_nil; ++i) {
        list = getRest(list);
    }
    return createNumber(i);
}

Object *f_reverse(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *list = getFirst(args);
    return reverse(list);
}

Object *f_append(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *arg = args, *list;
    Object *acum = obj_nil;

    while (arg != obj_nil) {
        list = getFirst(arg);

        while (list != obj_nil) {
            acum = createCons(getFirst(list), acum);
            list = getRest(list);
        }

        arg = getRest(arg);
    }
    return reverse(acum);
}

Object *f_null(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getFirst(args) == obj_nil) ? obj_t : obj_nil;
}

Object *f_list(Object *lst, Object *env) {
    // (list 1 2 3 4) => (1 2 3 4)
    return evalList(lst, env);
}

Object *f_list_star(Object *lst, Object *env) {
    // (list* 1 2 3 4) => (1 2 3 . 4)
    Object *args = evalList(lst, env);
    Object *lastCons, *prevValue, *iter = args;

    while (1) {
        prevValue = getFirst(iter);
        lastCons = getRest(iter);

        if (getRest(lastCons) == obj_nil) {
            if (lastCons->type == CONS) {
                iter->cdr = getFirst(lastCons);
                break;
            } else {
                // list only has 1 item
                return prevValue;
            }
        }
        iter = getRest(iter);
    }

    return args;
}

Object *f_eq(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (objEquals(getElem(args, 0), getElem(args, 1))) ? obj_t : obj_nil;
}

Object *f_plus(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return createNumber(a + b);
}

Object *f_minus(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return createNumber(a - b);
}

Object *f_times(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return createNumber(a * b);
}

Object *f_div(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    if (b == 0) {
        kdebug("Divided by zero\n");
        THROW(EXCEPTION_DIVIDED_BY_ZERO);
    }
    return createNumber(a / b);
}

Object *f_rem(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    if (b == 0) {
        kdebug("Divided by zero\n");
        THROW(EXCEPTION_DIVIDED_BY_ZERO);
    }
    return createNumber(a % b);
}

Object *f_equals(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a == b ? obj_t : obj_nil;
}

Object *f_greater(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a > b ? obj_t : obj_nil;
}

Object *f_less(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a < b ? obj_t : obj_nil;
}

Object *f_greater_equals(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a >= b ? obj_t : obj_nil;
}

Object *f_less_equals(Object *lst, Object *env) {
    Object *args = evalList(lst, env);

    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a <= b ? obj_t : obj_nil;
}

Object *f_and(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return ((getElem(args, 0) != obj_nil) && (getElem(args, 1) != obj_nil)) ? obj_t : obj_nil;
}

Object *f_or(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return ((getElem(args, 0) != obj_nil) || (getElem(args, 1) != obj_nil)) ? obj_t : obj_nil;
}

Object *f_not(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0) == obj_nil) ? obj_t : obj_nil;
}

Object *f_numberp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0)->type == NUMBER) ? obj_t : obj_nil;
}

Object *f_evenp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getNumber(getElem(args, 0)) % 2 == 0) ? obj_t : obj_nil;
}

Object *f_oddp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getNumber(getElem(args, 0)) % 2 != 0) ? obj_t : obj_nil;
}

Object *f_dotimes(Object *lst, Object *env) {
    // (dotimes (i 10) (println i))
    Object *iterator = getElem(getFirst(lst), 0);
    Object *limit = getElem(getFirst(lst), 1);

    int times = getNumber(eval(limit, env));
    Object *code = getRest(lst), *tmpEnv;

    if (iterator->type != SYMBOL) {
        kdebug("Error not a symbol: ");
        printObj(iterator);
        kdebug("\n");
        THROW(EXCEPTION_NOT_A_SYMBOL);
        return obj_nil;
    }

    for (int i = 0; i < times; ++i) {
        tmpEnv = createCons(createCons(iterator, createNumber(i)), env);
        evalList(code, tmpEnv);
    }
    return obj_nil;
}

Object *f_dolist(Object *lst, Object *env) {
    // (dolist (x '(1 5 10 55)) (println x))
    Object *iterator = getElem(getFirst(lst), 0);
    Object *list = eval(getElem(getFirst(lst), 1), env);
    Object *code = getRest(lst), *tmpEnv;

    if (iterator->type != SYMBOL) {
        kdebug("Error not a symbol: ");
        printObj(iterator);
        kdebug("\n");
        THROW(EXCEPTION_NOT_A_SYMBOL);
        return obj_nil;
    }

    while (list != obj_nil) {
        tmpEnv = createCons(createCons(iterator, getFirst(list)), env);
        evalList(code, tmpEnv);
        list = getRest(list);
    }

    return obj_nil;
}

static Object *f_map_aux(Object *code, Object *list, Object *env) {
    if (list == obj_nil) return obj_nil;
    Object *newValue = callFunction(code, createCons(getFirst(list), obj_nil), env);

    return createCons(newValue, f_map_aux(code, getRest(list), env));
}

Object *f_map(Object *lst, Object *env) {
    // (map println '(1 2 5 8)) -> (10 20 50 80)
    Object *args = evalList(lst, env);
    Object *code = getElem(args, 0);
    Object *list = getElem(args, 1);

    return f_map_aux(code, list, env);
}

static Object *f_filter_aux(Object *code, Object *list, Object *env) {
    if (list == obj_nil) return obj_nil;
    Object *cond = callFunction(code, createCons(getFirst(list), obj_nil), env);

    if (cond != obj_nil) {
        return createCons(getFirst(list), f_filter_aux(code, getRest(list), env));
    } else {
        return f_filter_aux(code, getRest(list), env);
    }
}

Object *f_filter(Object *lst, Object *env) {
    // (filter evenp '(1 2 3 4)) -> (2 4)
    Object *args = evalList(lst, env);
    Object *code = getElem(args, 0);
    Object *list = getElem(args, 1);

    return f_filter_aux(code, list, env);
}

Object *f_apply(Object *lst, Object *env) {
    // (apply '+ '(1 2)) -> 3
    Object *func = eval(getFirst(lst), env);
    Object *funcArgs = eval(getElem(lst, 1), env);

    if (func->type == SYMBOL) {
        func = lookupSymbol(func, env);
    }

    return callFunction(func, funcArgs, env);
}

Object *f_funcall(Object *lst, Object *env) {
    // (funcall '+ 1 2) =>  3
    Object *func = eval(getFirst(lst), env);
    Object *funcArgs = getRest(lst);

    if (func->type == SYMBOL) {
        func = lookupSymbol(func, env);
    }

    return callFunction(func, funcArgs, env);
}

Object *f_pipe(Object *lst, Object *env) {
    // (pipe 1 (+ 1) (* 2)) =>  4
    Object *aux;

    aux = eval(getFirst(lst), env);
    lst = getRest(lst);

    while (lst != obj_nil) {
        Object *call = getFirst(getFirst(lst));
        Object *args = createCons(aux, getRest(getFirst(lst)));

        aux = callFunction(eval(call, env), evalList(args, env), env);
        lst = getRest(lst);
    }

    return aux;
}

Object *f_println(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    printObj(getFirst(args));
    kdebug("\n");
    return obj_nil;
}

Object *f_print(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    printObj(getFirst(args));
    return obj_nil;
}

Object *f_clear(Object *lst, Object *env) {
    monitor_clear(motherboard_get_monitor());
    return obj_nil;
}

Object *f_sleep(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int ticks = getNumber(getFirst(args)) & 0xFF;
    motherboard_sleep((Byte) ticks);
    return obj_nil;
}


Object *f_help(Object *lst, Object *env) {
    kdebug("Avaliable commands: \n");
    for (Object *cons = env; cons != obj_nil; cons = getRest(cons)) {
        Object *pair = getFirst(cons);
        printObj(getFirst(pair));
        kdebug(" ");
    }
    kdebug("\n");
    return obj_nil;
}

void fn_init() {
    obj_nil = createSymbol("nil");
    obj_env = createCons(createCons(obj_nil, obj_nil), obj_nil);
    obj_t = registerSymbol("t");
    obj_quote = registerFunc("quote", f_quote);
    obj_progn = registerFunc("progn", f_progn);

    registerFunc("`", f_quasiquote);
    registerFunc("debug", f_debug);
    registerFunc("if", f_if);
    registerFunc("lambda", f_lambda);
    registerFunc("define", f_define);
    registerFunc("set!", f_set);
    registerFunc("defun", f_defun);
    registerFunc("defmacro", f_defmacro);
    registerFunc("car", f_car);
    registerFunc("cdr", f_cdr);
    registerFunc("cadr", f_cadr);
    registerFunc("atom", f_atom);
    registerFunc("first", f_car);
    registerFunc("rest", f_cdr);
    registerFunc("nth", f_nth);
    registerFunc("cons", f_cons);
    registerFunc("length", f_length);
    registerFunc("append", f_append);
    registerFunc("reverse", f_reverse);
    registerFunc("null", f_null);
    registerFunc("list", f_list);
    registerFunc("list*", f_list_star);
    registerFunc("eq", f_eq);
    registerFunc("+", f_plus);
    registerFunc("-", f_minus);
    registerFunc("*", f_times);
    registerFunc("/", f_div);
    registerFunc("%", f_rem);
    registerFunc("=", f_equals);
    registerFunc(">", f_greater);
    registerFunc("<", f_less);
    registerFunc(">=", f_greater_equals);
    registerFunc("<=", f_less_equals);
    registerFunc("and", f_and);
    registerFunc("or", f_or);
    registerFunc("not", f_not);
    registerFunc("numberp", f_numberp);
    registerFunc("evenp", f_evenp);
    registerFunc("oddp", f_oddp);
    registerFunc("dotimes", f_dotimes);
    registerFunc("dolist", f_dolist);
    registerFunc("map", f_map);
    registerFunc("filter", f_filter);
    registerFunc("apply", f_apply);
    registerFunc("funcall", f_funcall);
    registerFunc("pipe", f_pipe);
    registerFunc("println", f_println);
    registerFunc("print", f_print);
    registerFunc("clear", f_clear);
    registerFunc("env", f_help);
    registerFunc("help", f_help);
    registerFunc("sleep", f_sleep);
}