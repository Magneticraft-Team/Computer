//
// Created by cout970 on 17/02/18.
//

#ifndef COMPUTER_COMMANDS_H
#define COMPUTER_COMMANDS_H

#include <types.h>

typedef String *ArgType;

typedef void (*Func0)(void);

typedef void (*Func1)(ArgType);

typedef void (*Func2)(ArgType, ArgType);

typedef void (*Func3)(ArgType, ArgType, ArgType);

struct Cmd {
    String name[32];
    String usage[47];
    Int argCount;
    union {
        Func0 func0;
        Func1 func1;
        Func2 func2;
        Func3 func3;
    };
};

void f_help();

void f_format();

void f_ls();

void f_cd(ArgType name);

void f_rm(ArgType name);

void f_mkdir(ArgType name);

void f_free();

void f_fs();

void f_cat(ArgType name);

void f_touch(ArgType name);

void f_write(ArgType name);

void f_update_disk();

void f_pastebin(ArgType file, ArgType code);

void f_update(ArgType file);

void f_quarry(ArgType file);

#endif //COMPUTER_COMMANDS_H
