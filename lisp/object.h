//
// Created by cout970 on 2017-08-12.
//

#ifndef MAGNETICRAFTCOMPUTER_OBJECT_H
#define MAGNETICRAFTCOMPUTER_OBJECT_H


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

#endif //MAGNETICRAFTCOMPUTER_OBJECT_H
