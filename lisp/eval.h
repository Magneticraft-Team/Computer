//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_EVAL_H
#define MAGNETICRAFTCOMPUTER_EVAL_H

#include "object.h"

Object *eval(Object *exp, Object *env);

Object *evalList(Object *exps, Object *env);

#endif //MAGNETICRAFTCOMPUTER_EVAL_H
