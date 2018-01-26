//
// Created by cout970 on 2017-07-10.
//

#ifndef DRIVER_TYPES_H
#define DRIVER_TYPES_H

#ifndef NULL
#define NULL ((void*)0x0)
#endif

#define IGNORED __attribute__((unused))

#define FALSE 0
#define TRUE (!FALSE)

typedef char Byte;
typedef char Char;
typedef short Short;
typedef int Int;

typedef char Boolean;
typedef char String;

typedef unsigned char UBite;
typedef unsigned short UShort;
typedef unsigned int UInt;

typedef Byte* ByteBuffer;
typedef void* Ptr;

#endif //DRIVER_TYPES_H
