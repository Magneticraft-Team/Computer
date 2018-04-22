//
// Created by cout970 on 2017-08-12.
//

#ifndef FORTH_DEPENDENCIES_H
#define FORTH_DEPENDENCIES_H
#ifdef DEBUG_ENV

#include <stdint.h>

#else

#include <glib/stddef.h>
#endif

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <math.h>
#include <fs/filesystem.h>

#define IGNORED __attribute__((unused))

#endif //FORTH_DEPENDENCIES_H
