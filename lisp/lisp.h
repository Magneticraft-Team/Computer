//
// Created by cout970 on 2017-09-06.
//

#ifndef MAGNETICRAFTCOMPUTER_LISP_H
#define MAGNETICRAFTCOMPUTER_LISP_H

#include "dependencies.h"

enum ObjectType {
    INT,    // number
    SYM,    // symbol
    CONS,   // construct (pair or list)
    PROC,   // procedure (lisp function)
    PRIMOP  // primitive operation (c function)
};

typedef struct obj {
    int hash;       // index of creation, unique per object
    int marked;     // used for garbage collection
    void *next;     // used for garbage collection

    enum ObjectType type;
    int line_num;           // line where it was created
    union {
        // type SYM
        const char *string;
        // type INT
        const int number;
        // type PRIMOP
        const struct obj *(*function)(struct obj*);
        // type CONS, PROC
        struct obj *data[1];    // dynamic length array, the contents depends on the type of the object
    };

} Object;

// Define Primop as Object* -> Object*
typedef Object *(*Primop)(Object *);

// CONSTRUCTORS

Object *createCons(Object *car, Object *cdr);

Object *createProc(Object *args, Object *code, Object *env);

Object *createPrimop(Primop func);

Object *createSymbol(const char *symbol);

Object *createInt(int value);

// ENVIRONMENT

// Adds a list of pairs to the environment
Object *multiple_extend(Object *env, Object *syms, Object *vals);

// Adds a pair (symbol, value) to the top environment
Object *extend_top(Object *sym, Object *val);

Object *extend_symbols(Object *op);

// Finds a pair (key, value) in list that matches the given 'key', returns nil if not found
Object *assoc(Object *key, Object *list);

Object *findSymbol(const char *name);

Object *getOrCreateSymbol(const char *name);

// EVAL

Object *eval(Object *exp, Object *env);

Object *evalList(Object *exps, Object *env);

// GARBAGE COLLECTOR

void markAll();

void mark(Object *obj);

void sweep();

void freeObj(Object *obj);

void freeSymbolString(const char *token);

// GETTERS

Object *getCar(Object *obj);

Object *getCdr(Object *obj);

Object *getFirst(Object *obj);

Object *getRest(Object *obj);

void setCdr(Object *a, Object *cdr);

Primop getPrimop(Object *obj);

int getInt(Object *obj);

Object *getProcArgs(Object *obj);

Object *getProcCode(Object *obj);

Object *getProcEnv(Object *obj);

char *getSymbolName(Object *obj);

int isNil(Object *obj);

Object *getElem(Object *list, int index);

// READER

void initReader();

void readLineBuffered();

char getNextChar();

void putNextChar(char a);

// TOKENIZER

enum TokenType {
    NUMBER, STRING, LEFT_PAREN, RIGHT_PAREN, QUOTE, DOT
};

void initTokenizer();

const char *createSymbolString();

void readToken();

void nextToken();

Object *readObj();

Object *readList();

// DEBUG

// recursive print
void printObjFormatted(Object *op);

// non-recursive print
void printObj(Object *obj);

// FUNCTIONS

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

Object *prim_network(Object *args IGNORED);

// GLOBALS

extern const char *objTypeNames[];

extern jmp_buf onError;
// current interpreter line
extern int line_num;
// total memory allocated
extern int total_malloc;
// total number of string allocated
extern int string_count;
// used to not print evaluation output
extern int write_output_flag;
// last object index, used to get the unique number of each object
extern int lastNum;

//used for garbage collection
extern Object *all_objects;

extern Object *all_symbols, *top_env;
extern Object *nil, *tee, *quote, *s_if, *s_lambda, *s_define, *s_setb, *s_begin, *s_defun;

#endif //MAGNETICRAFTCOMPUTER_LISP_H
