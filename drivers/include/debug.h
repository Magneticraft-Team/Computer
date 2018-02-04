//
// Created by cout970 on 15/01/18.
//

#ifndef MAGNETICRAFTCOMPUTER_DEBUG_H
#define MAGNETICRAFTCOMPUTER_DEBUG_H

#include "base.h"

#ifdef DEBUG_ENV
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define kdebug(...) printf(__VA_ARGS__)
#else
#include "kprint.h"
#define kdebug(...) kprint(__VA_ARGS__)
#endif

#endif //MAGNETICRAFTCOMPUTER_DEBUG_H
