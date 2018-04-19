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
        TRY {
            obj = pr_parse();

            if (obj == NULL) {
                break;
            }

            result = eval(obj, obj_env);

            if (result != obj_nil) {
                printObj(result);
                kdebug("\n");
            }
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
        case KEYWORD:
        case NATIVE_FUN:
        case FUNC:
        case MACRO:
            return exp;
        case SYMBOL:
            return lookupSymbol(exp, env);
        case CONS:
            proc = eval(getFirst(exp), env);
            args = getRest(exp);

            return callFunction(proc, args, env);
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

    if (func->type == MACRO) {
        Object *newEnv = addArgsToEnv(func->macro_env, func->macro_args, evalList(args, env));
        return eval(expandMacro(func->code, newEnv), env);
    }

    if (func->type == NATIVE_FUN) {
        return func->function(args, env);
    }

    if (func->type == FUNC) {
        Object *newEnv = addArgsToEnv(func->env, func->args, evalList(args, env));
        Object *exp = createCons(obj_progn, func->code);
        return eval(exp, newEnv);
    }

    kdebug("Not a function: ");
    printObj(func);
    kdebug("\n");
    THROW(EXCEPTION_NOT_A_FUNCTION);
    return obj_nil;
}

Object *evalList(Object *exps, Object *env) {

    Object *result = obj_nil;

    while (exps != obj_nil) {
        result = createCons(eval(getFirst(exps), env), result);
        exps = getRest(exps);
    }

    return reverse(result);
}

Object *expandMacro(Object *code, Object *env) {

    if (code->type != CONS) {
        return code;
    }

    if (getFirst(code)->type == SYMBOL && symbolEquals(getFirst(code), obj_unquote)) {
        return eval(getElem(code, 1), env);
    }

    Object *start = obj_nil;
    Object **lastCons = &start;

    for (Object *lines = code; lines != obj_nil; lines = getRest(lines)) {

        Object *line = getFirst(lines);
        Object *expanded = expandMacro(line, env);

        Object *newCons = createCons(expanded, obj_nil);
        *lastCons = newCons;
        lastCons = &newCons->cdr;
    }

    return start;
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