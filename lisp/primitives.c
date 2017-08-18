//
// Created by cout970 on 2017-08-12.
//

#include "primitives.h"
#include "constructors.h"
#include "getters.h"
#include "globals.h"
#include "print.h"
#include "gc.h"


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

Object *prim_mod(Object *args) {
    int prod = getInt(getFirst(args));
    args = getRest(args);
    while (!isNil(args)) {
        prod %= getInt(getFirst(args));
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

Object *prim_num_eq(Object *args) {
    return getInt(getFirst(args)) == getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_cons(Object *args) { return createCons(getFirst(args), getFirst(getRest(args))); }

Object *prim_car(Object *args) { return getFirst(getFirst(args)); }

Object *prim_cdr(Object *args) { return getRest(getFirst(args)); }


/*** Helpers ***/

Object *prim_print(Object *args) {
    while (!isNil(args)) {
        printObjFormatted(getFirst(args));
        args = getRest(args);
        printf(" ");
    }
    printf("\n");
    return nil;
}

Object *prim_clear(Object *args IGNORED) {
    clear_screen();
    monitor_set_cursor_pos_y(motherboard_get_monitor(), 0);
    write_output_flag = 1;
    return nil;
}

Object *prim_free(Object *args) {
    if (!isNil(args)) { printf(""); }
    int used = (int) malloc(4);
    free((void *) used);
    register int *sp asm ("sp");

    int free = motherboard_get_memory_size() - used - (0xFFFF - (int)sp);
    printf("%d bytes used, %d bytes free, malloc ptr: %d, string count: %d, sp: %d\n",
           total_malloc, free, used, string_count, (int) sp);
    return nil;
}

Object *prim_env(Object *args IGNORED) {
    Object *symbolList;

    for (symbolList = getRest(top_env); !isNil(symbolList); symbolList = getRest(symbolList)) {
        printf("%s ", getSymbolName(getFirst(getFirst(symbolList))));
    }
    return nil;
}

Object *prim_symbols(Object *args IGNORED) {
    Object *symbolList;

    for (symbolList = all_symbols; !isNil(symbolList); symbolList = getRest(symbolList)) {
        printf("%s ", getSymbolName(getFirst(symbolList)));
    }
    return nil;
}

Object *prim_to_int(Object *args) {
    return createInt(getInt(args));
}

Object *prim_not(Object *args) {
    return isNil(args) ? tee : nil;
}

Object *prim_gc(Object *args IGNORED) {
    markAll();
    sweep();
    malloc_compact();
    return nil;
}

Object *prim_debug(Object *args IGNORED) {
    Object *obj;

    for (obj = all_objects; obj != NULL; obj = (Object *) obj->next) {
        printObj(obj);
        putchar(' ');
    }
    putchar('\n');
    return nil;
}

Object *prim_read32(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    int32_t *ptr = (int32_t *) getInt(addr);
    return createInt(*ptr);
}

Object *prim_read16(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    int16_t *ptr = (int16_t *) getInt(addr);
    return createInt(*ptr);
}

Object *prim_read8(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    int8_t *ptr = (int8_t *) getInt(addr);
    return createInt(*ptr);
}

Object *prim_write32(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    Object *value = getFirst(getRest(args));
    int32_t *ptr = (int32_t *) getInt(addr);
    *ptr = getInt(value);
    return nil;
}

Object *prim_write16(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    Object *value = getFirst(getRest(args));
    int16_t *ptr = (int16_t *) getInt(addr);
    *ptr = (int16_t) getInt(value);
    return nil;
}

Object *prim_write8(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    Object *value = getFirst(getRest(args));
    int8_t *ptr = (int8_t *) getInt(addr);
    *ptr = (int8_t) getInt(value);
    return nil;
}

Object *prim_and(Object *args) {
    if (isNil(args)) return nil;
    Object *a = getFirst(args);
    Object *b = getFirst(getRest(args));
    return createInt(getInt(a) & getInt(b));
}

Object *prim_or(Object *args) {
    if (isNil(args)) return nil;
    Object *a = getFirst(args);
    Object *b = getFirst(getRest(args));
    return createInt(getInt(a) | getInt(b));
}

Object *prim_xor(Object *args) {
    if (isNil(args)) return nil;
    Object *a = getFirst(args);
    Object *b = getFirst(getRest(args));
    return createInt(getInt(a) ^ getInt(b));
}