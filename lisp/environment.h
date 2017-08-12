//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_ENVIROMENT_H
#define MAGNETICRAFTCOMPUTER_ENVIROMENT_H

#include "object.h"

// Adds a list of pairs to the environment
Object *multiple_extend(Object *env, Object *syms, Object *vals);

// Adds a pair (symbol, value) to the top environment
Object *extend_top(Object *sym, Object *val);

Object *extend_symbols(Object *op);

// Finds a pair (key, value) in list that matches the given 'key', returns nil if not found
Object *assoc(Object *key, Object *list);

Object *findSymbol(const char *name);

Object *getOrCreateSymbol(const char *name);

#endif //MAGNETICRAFTCOMPUTER_ENVIROMENT_H
