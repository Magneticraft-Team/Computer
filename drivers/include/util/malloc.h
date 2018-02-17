//
// Created by cout970 on 2017-07-23.
//

#include <types.h>

#ifndef DRIVER_MALLOC_H
#define DRIVER_MALLOC_H

void free(Ptr ptr);

Ptr malloc(UInt size);

void initHeap(Ptr start, UInt size);

#endif //DRIVER_MALLOC_H
