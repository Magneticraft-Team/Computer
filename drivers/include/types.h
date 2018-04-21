//
// Created by cout970 on 2017-07-10.
//
// Provides basic types for headers
// Remember, you are not forced to use them, feel free to define them as i8, i16... or int8_t, int16_t... for you code

#ifndef DRIVER_TYPES_H
#define DRIVER_TYPES_H

#ifndef NULL
#define NULL ((void*)0x0)
#endif

#ifndef EOF
# define EOF (-1)
#endif

#define IGNORED __attribute__((unused))
#define PACKED __attribute__((packed))

#define ERROR (-1)
#define SUCCESS 0

#define FALSE 0
#define TRUE (!FALSE)

typedef char Byte;
typedef char Char;
typedef short Short;
typedef int Int;

typedef char Boolean;
typedef char String;

typedef unsigned char UByte;
typedef unsigned short UShort;
typedef unsigned int UInt;

typedef Byte* ByteBuffer;
typedef void Any;
typedef void* Ptr;

#endif //DRIVER_TYPES_H
