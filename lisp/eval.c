//
// Created by cout970 on 2017-08-12.
//

#include "eval.h"
#include "getters.h"
#include "constructors.h"
#include "globals.h"
#include "environment.h"
#include "print.h"


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
                printf("Unbound symbol '");
                printObjFormatted(exp);
                printf("'\n");
                longjmp(onError, 2);
                return nil;
            }
            return getRest(tmp);
        case CONS:
            if (getFirst(exp) == s_if) { // IF
                //(if (exp) ifTrue ifFalse)
                ifExp = getFirst(getRest(exp));
                if (eval(ifExp, env) != nil)
                    return eval(getElem(exp, 2), env);
                else
                    return eval(getElem(exp, 3), env);
            }

            if (getFirst(exp) == s_lambda) { // LAMBDA
                // (lambda (args...) code)
                return createProc(getElem(exp, 1), getRest(getRest(exp)), env);
            }

            if (getFirst(exp) == quote) { // QUOTE '
                // '(listElements...)
                return getFirst(getRest(exp));
            }

            if (getFirst(exp) == s_define) { // DEFINE
                // (define symbol value)
                return (extend_top(getElem(exp, 1), eval(getElem(exp, 2), env)));
            }

            if (getFirst(exp) == s_setb) { // SET!
                // (set! symbol value)
                Object *pair = assoc(getElem(exp, 1), env);
                Object *newval = eval(getElem(exp, 2), env);
                setCdr(pair, newval);
                return newval;
            }

            if (getFirst(exp) == s_begin) { // BEGIN
                // (begin
                //          line1
                //          line2
                //          ...)
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

            if (getFirst(exp) == s_defun) { // DEFUN
                // (defun symbol (args...) code)
                extend_top(getElem(exp, 1), createProc(getElem(exp, 2), getRest(getRest(getRest(exp))), env));
                return nil;
            }

//            if (getFirst(exp) == s_let) { // DEFUN
//                // (let ((key value) (key value)...))
//                env = multiple_extend_pairs(env, getRest(exp));
//                return nil;
//            }
//            Object *multiple_extend(Object *env, Object *syms, Object *vals) {
//
//                if (isNil(syms)) return env;
//
//                Object *pair = createCons(getFirst(syms), getFirst(vals));
//                Object *newEnv = createCons(pair, env);
//                // continue until syms is empty
//                return multiple_extend(newEnv, getRest(syms), getRest(vals));
//            }

            proc = eval(getFirst(exp), env);
            vals = evalList(getRest(exp), env);

            if (proc->type == PRIMOP) // C FUNCTION
                return (*getPrimop(proc))(vals);

            if (proc->type == PROC) { // COMPOSITE FUNCTION
                env = multiple_extend(getProcEnv(proc), getProcArgs(proc), vals);
                exp = createCons(s_begin, getProcCode(proc));
                goto eval_start;
            }
            printf("Not a function: ");
            printObjFormatted(proc);
            printf("\n");
            longjmp(onError, 1);
            return nil;
    }
    /* Not reached */
    return exp;
}

Object *evalList(Object *exps, Object *env) {
    if (exps == nil) return nil;
    return createCons(eval(getFirst(exps), env),
                      evalList(getRest(exps), env));
}