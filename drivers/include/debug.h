//
// Created by cout970 on 15/01/18.
//
// this file defines the kdebug macro, used for debug
// If the code is compiled for x86, it will print in the console
// If is compiled to mips and:
// - USE_DEBUG_LOG is defined it will print in the minecraft log
// - USE_DEBUG_LOG is not defined, it will print in the monitor

#ifndef COMPUTER_DEBUG_H
#define COMPUTER_DEBUG_H

#include "base.h"

#ifdef DEBUG_ENV
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define kdebug(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
#include "kprint.h"
#define kdebug(...) kprint(__VA_ARGS__)
#endif

#endif //COMPUTER_DEBUG_H
