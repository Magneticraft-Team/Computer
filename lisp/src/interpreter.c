//
// Created by cout970 on 19/09/17.
//
// TODO http://clhs.lisp.se/Body/26_glo_c.htm#conforming_implementation

#include <debug.h>
#include <glib/setjmp.h>
#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/functions.h"
#include "../include/exception.h"
#include "../include/gc.h"

void in_init() {
    fn_init();
}

void in_run() {

    Object *obj, *result;

    while (1) {
        obj = pr_parse();

        if (obj == NULL) {
            break;
        }

        TRY {
#ifdef DEBUG_ENV
            kdebug("Input: ");
            printObj(obj);
            kdebug("\n");
#endif
            result = eval(obj, obj_env);
            printObj(result);
            kdebug("\n");
        } CATCH {
            // No-op
        };
        gc(obj_env);
    }
}

Object *eval(Object *exp, Object *env) {
    Object *tmp, *proc, *args;

    if (exp == obj_nil) return obj_nil;

//    printObj(exp);
//    kdebug("\n");

    switch (exp->type) {
        case STRING:
        case NUMBER:
        case NATIVE_FUN:
        case FUNC:
            return exp;
        case SYMBOL:
            return lookupSymbol(exp, env);
        case CONS:
            args = getRest(exp);
            proc = eval(getFirst(exp), env);

            return callFunction(proc, args, env);
        case MACRO:
            // TODO
            break;
    }
    /* Not reached */
    return obj_nil;
}

Object *lookupSymbol(Object *sym, Object *env) {
    Object *tmp = assoc(sym, env);

    if (tmp == obj_nil) {
        kdebug("Unbound symbol '");
        printObj(sym);
        kdebug("'\n");

        THROW(EXCEPTION_UNBOUND_SYMBOL);
        return obj_nil;
    }
    return getRest(tmp);
}

Object *callFunction(Object *func, Object *args, Object *env) {

    if (func->type == NATIVE_FUN) {
        return func->function(args, env);

    } else if (func->type == FUNC) {
        Object *newEnv = addArgsToEnv(func->env, func->args, evalList(args, env));
        Object *exp = createCons(obj_progn, func->code);
        return eval(exp, newEnv);

    } else {
        kdebug("Not a function: ");
        printObj(func);
        kdebug("\n");
        THROW(EXCEPTION_NOT_A_FUNCTION);
        return obj_nil;
    }
}

Object *evalList(Object *exps, Object *env) {

    Object *result = obj_nil;

    while (exps != obj_nil) {
        result = createCons(eval(getFirst(exps), env), result);
        exps = getRest(exps);
    }

    return reverse(result);
}

Object *addArgsToEnv(Object *env, Object *syms, Object *vals) {

    if (syms == obj_nil) return env;

    Object *pair = createCons(getFirst(syms), getFirst(vals));
    Object *newEnv = createCons(pair, env);
    // continue until syms is empty
    return addArgsToEnv(newEnv, getRest(syms), getRest(vals));
}

Object *assoc(Object *key, Object *env) {
    Object *pair;
    while (env != obj_nil) {
        pair = getFirst(env);
        if (symbolEquals(getFirst(pair), key)) return pair;
        env = getRest(env);
    }
    return obj_nil;
}

Object *extendEnv(Object *sym, Object *val) {
    Object *pair = createCons(sym, val);
    obj_env->cdr = createCons(pair, getRest(obj_env));
    return val;
}

Object *reverse(Object *list) {
    Object *acum = obj_nil;

    while (list != obj_nil) {
        acum = createCons(getFirst(list), acum);
        list = getRest(list);
    }

    return acum;
}