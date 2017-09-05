//
// Created by cout970 on 2017-08-12.
//

#include "lisp.h"

/*** Environment ***/

// Adds a list of pairs to the environment
Object *multiple_extend(Object *env, Object *syms, Object *vals) {

    if (isNil(syms)) return env;

    Object *pair = createCons(getFirst(syms), getFirst(vals));
    Object *newEnv = createCons(pair, env);
    // continue until syms is empty
    return multiple_extend(newEnv, getRest(syms), getRest(vals));
}

// Adds a pair (symbol, value) to the top environment
Object *extend_top(Object *sym, Object *val) {
    Object *pair = createCons(sym, val);

    //insert node
    setCdr(top_env, createCons(pair, getRest(top_env)));
    return val;
}

Object *extend_symbols(Object *op) {
    all_symbols = createCons(op, all_symbols);
    return op;
}

// Finds a pair (key, value) in list that matches the given 'key', returns nil if not found
Object *assoc(Object *key, Object *list) {
    if (isNil(list)) return nil;

    if (getFirst(getFirst(list)) == key) return getFirst(list);
    return assoc(key, getRest(list));
}

Object *findSymbol(const char *name) {
    Object *it;
    int count = 0;

    for (it = all_symbols; !isNil(it); it = getRest(it)) {
        if (strcmp(name, getSymbolName(getFirst(it))) == 0) {
            return it;
        }
        count++;
    }
    return nil;
}

Object *getOrCreateSymbol(const char *name) {
    Object *op = findSymbol(name);

    if (!isNil(op)) {
        return getFirst(op);
    }
    return extend_symbols(createSymbol(name));
}