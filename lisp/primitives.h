//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_PRIMITIVES_H
#define MAGNETICRAFTCOMPUTER_PRIMITIVES_H

#include "object.h"
#include "dependencies.h"

/*** Primitives ***/

Object *prim_sum(Object *args);

Object *prim_sub(Object *args);

Object *prim_prod(Object *args);

Object *prim_divide(Object *args);

Object *prim_mod(Object *args);

Object *prim_gt(Object *args);

Object *prim_lt(Object *args);

Object *prim_ge(Object *args);

Object *prim_le(Object *args);

Object *prim_num_eq(Object *args);

Object *prim_cons(Object *args);

Object *prim_car(Object *args);

Object *prim_cdr(Object *args);


/*** Helpers ***/

Object *prim_print(Object *args);

Object *prim_clear(Object *args IGNORED);

Object *prim_free(Object *args);

Object *prim_env(Object *args IGNORED);

Object *prim_symbols(Object *args IGNORED);

Object *prim_to_int(Object *args);

Object *prim_not(Object *args);

Object *prim_gc(Object *args IGNORED);

Object *prim_debug(Object *args IGNORED);

Object *prim_read32(Object *args);

Object *prim_read16(Object *args);

Object *prim_read8(Object *args);

Object *prim_write32(Object *args);

Object *prim_write16(Object *args);

Object *prim_write8(Object *args);

Object *prim_and(Object *args);

Object *prim_or(Object *args);

Object *prim_xor(Object *args);

#endif //MAGNETICRAFTCOMPUTER_PRIMITIVES_H
