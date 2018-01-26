//
// Created by cout970 on 19/01/18.
//

#ifndef MAGNETICRAFTCOMPUTER_KPRINT_H
#define MAGNETICRAFTCOMPUTER_KPRINT_H

#include "types.h"

void kputchar(Char c);

Int kprintn(Int num, Int base);

Int kprintun(UInt num, Int base);

Int kprints(const String *str);

Int kprintsn(const String *str, Int count);

Int kprint(const String *str, ...);

#endif //MAGNETICRAFTCOMPUTER_KPRINT_H
