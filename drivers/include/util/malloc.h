//
// Created by cout970 on 2017-07-23.
//

#include <types.h>

#ifndef DRIVER_MALLOC_H
#define DRIVER_MALLOC_H

void free(Any* ptr);

Any* malloc(UInt size);

Any* realloc(Any* ptr, UInt size);

void initHeap(Any* start, UInt size);

#endif //DRIVER_MALLOC_H
